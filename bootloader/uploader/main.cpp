/*
 * This is a derivative of SysExBoot
 *
 * Modified by Brian Richardson for Jabrudian Industries
 *
 * >>>>>>>>>>>>>> origional header >>>>>>
 *
 * "A sysex bootloader for avr chips."
 * Copyright 2010 Alex Norman
 *
 * This file is part of SysexBoot.
 *
 *
 * SysexBoot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SysexBoot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SysexBoot.  If not, see <http://www.gnu.org/licenses/>.
 *
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
#include <vector>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "portmidi.h"
#include "porttime.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "sysex_tools.h"

#include "midibootcommon.h"

#define TIMEOUT_MS 3000

#define CHUNK_SIZE 48

/*
 * a sysex packet for a page_fill looks like:
 * MIDI_SYSEX_START
 * midiboot_sysex_id[]
 * MIDIBOOT_FILLPAGE
 * packed_addr+data[<=35] (unpacks to 2 addr bytes + 28 or fewer data bytes)
 * MIDI_SYSEX_STOP
 */

using std::cout;
using std::cerr;
using std::endl;
using std::hex;
using std::dec;

void simple_bit_unpack(uint8_t * unpacked, const uint8_t * source,
		       uint16_t length)
{
	for (int i = 0; i < length; i += 2) {
		unpacked[i / 2] = (source[i] << 4) | source[i + 1];
	}
}

uint16_t simple_bit_packed_length(uint16_t unpacked_length)
{
	return unpacked_length * 2;
}

uint16_t simple_bit_pack(uint8_t * packed, const uint8_t * source,
			 uint16_t length)
{
	for (int i = 0; i < length; i++) {
		packed[i * 2] = source[i] >> 4;
		packed[i * 2 + 1] = source[i] & 0xF;
	}
	return length * 2;
}

int getMissingLink(bool input)
{
	for (int i = 0; i < Pm_CountDevices(); i++) {
		const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
		if ((info->input && input) || (info->output && !input)) {
			if (strncmp(info->name, "Missing Link", 12) == 0) {
				return i;
			}
		}
	}
	return -1;
}

void printMidiDevices(bool input)
{
	for (int i = 0; i < Pm_CountDevices(); i++) {
		const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
		if (info->input && input)
			cout << i << ", " << info->interf << ", " << info->
			    name << endl;
		if (info->output && !input)
			cout << i << ", " << info->interf << ", " << info->
			    name << endl;
		//printf("%d: %s, %s\n", i, info->interf, info->name);
	}
}

void MLCommand(PmStream * midi_in, PmStream * midi_out, long timeout,
	       unsigned char command)
{
	// SYSEX BEGIN, EDU, MLNK, 3=enter bootloader, SYSEX END
	unsigned char requestMsg[] =
	    { 0xF0, 0x7D, 0x4D, 0x4C, 0x4E, 0x4B, command, 0xF7 };
	//send the message
	PmError error = Pm_WriteSysEx(midi_out, 0, requestMsg);
	if (error) {
		cerr << "error writing ENTER BOOTLOADER msg: " <<
		    Pm_GetErrorText(error) << endl;
		exit(-1);
	}
	long startTime = Pt_Time();
	while (Pt_Time() - startTime < timeout) ;
}

int getPageSize(PmStream * midi_in, PmStream * midi_out, long timeout)
{
	PmEvent msg;
	bool inSysexMessage;
	unsigned int sysexByteCount;
	long startTime = Pt_Time();
	int pageSize = 0;
	uint8_t buf[3];
	uint8_t bufout[3];

	//write get page size message
	unsigned char requestMsg[64];
	memset(&requestMsg, 0, 64);
	requestMsg[0] = MIDI_SYSEX_START;
	for (unsigned int i = 0; i < MIDIBOOT_SYSEX_ID_LEN; i++)
		requestMsg[i + 1] = midiboot_sysex_id[i];
	requestMsg[MIDIBOOT_SYSEX_ID_LEN + 1] = MIDIBOOT_GETPAGESIZE;
	requestMsg[MIDIBOOT_SYSEX_ID_LEN + 2] = MIDI_SYSEX_STOP;
	//send the message
	PmError error = Pm_WriteSysEx(midi_out, 0, requestMsg);
	if (error) {
		cerr << "error writing GET PAGE SIZE msg: " <<
		    Pm_GetErrorText(error) << endl;
		exit(-1);
	}
	//allow for a second timeout
	while (startTime + timeout > Pt_Time()) {
		unsigned int count = Pm_Read(midi_in, &msg, 1);
		if (count != 0) {
			for (unsigned int i = 0; i < 4; i++) {
				unsigned char inByte =
				    (msg.message >> (i * 8)) & 0xFF;
				if (inByte == MIDI_SYSEX_START) {
					sysexByteCount = 0;
					inSysexMessage = true;
				} else if (inSysexMessage) {
					if (inByte == MIDI_SYSEX_STOP) {
						simple_bit_unpack(bufout, buf,
								  4);
						pageSize =
						    (bufout[0] << 8) |
						    bufout[1];
						return pageSize;
					} else
					    if ((sysexByteCount <
						 MIDIBOOT_SYSEX_ID_LEN)) {
						//make sure that our id matches
						if (inByte ==
						    midiboot_sysex_id
						    [sysexByteCount])
							sysexByteCount++;
						else
							inSysexMessage = false;
					} else if (sysexByteCount ==
						   MIDIBOOT_SYSEX_ID_LEN) {
						if (inByte !=
						    MIDIBOOT_GETPAGESIZE)
							inSysexMessage = false;
						else
							sysexByteCount++;
					} else if (sysexByteCount <
						   (MIDIBOOT_SYSEX_ID_LEN +
						    5)) {
						unsigned int index =
						    (sysexByteCount -
						     MIDIBOOT_SYSEX_ID_LEN - 1);
						buf[index] = inByte;
						sysexByteCount++;
					} else
						inSysexMessage = false;
				}
			}
		}
	}
	cout << endl;
	return 0;
}

bool waitForAck(PmStream * midi_in, long timeout)
{
	PmEvent msg;
	bool inSysexMessage;
	unsigned int sysexByteCount;
	long startTime = Pt_Time();
	//allow for a second timeout
	while (startTime + timeout > Pt_Time()) {
		unsigned int count = Pm_Read(midi_in, &msg, 1);
		if (count != 0) {
			for (unsigned int i = 0; i < 4; i++) {
				unsigned char inByte =
				    (msg.message >> (i * 8)) & 0xFF;
				if (inByte == MIDI_SYSEX_START) {
					sysexByteCount = 0;
					inSysexMessage = true;
				} else if (inSysexMessage) {
					if (sysexByteCount ==
					    MIDIBOOT_SYSEX_ID_LEN) {
						if (inByte == MIDI_SYSEX_STOP) {
							return true;
						} else
							inSysexMessage = false;
					} else
					    if ((sysexByteCount <
						 MIDIBOOT_SYSEX_ID_LEN)
						&& inByte ==
						midiboot_sysex_id
						[sysexByteCount])
						sysexByteCount++;
					else
						inSysexMessage = false;
				}
			}
		}
	}
	return false;
}

void writeSysexPrefix(std::vector < unsigned char >*sysexMesg)
{
	sysexMesg->push_back(MIDI_SYSEX_START);
	for (unsigned int i = 0; i < MIDIBOOT_SYSEX_ID_LEN; i++)
		sysexMesg->push_back(midiboot_sysex_id[i]);
}

int main(int argc, char *argv[])
{
	std::vector < unsigned char >inputData;
	std::vector < std::vector < unsigned char >*>pages;
	std::vector < std::vector < unsigned char >*>sysexMessages;
	std::vector < unsigned char >packedPage;
	unsigned char tmp;
	int id;
	std::ifstream inFile;
	struct stat stFileInfo;
	PmStream *midi_in;
	PmStream *midi_out;
	unsigned int pageSize = 0;
	unsigned short numPages = 0;
	const PmDeviceInfo *info;

	//start our time
	Pt_Start(1, 0, 0);

	//test args
	if (argc != 2) {
		cout << "usage:" << endl;
		cout << argv[0] << " binfiletoupload" << endl;
		exit(-1);
	}
	if (stat(argv[1], &stFileInfo) != 0 || !S_ISREG(stFileInfo.st_mode)) {
		cerr << "file does not exist or is not a regular file" << endl;
		exit(-1);
	}

	printMidiDevices(true);
	printMidiDevices(false);

	id = getMissingLink(true);
	if (id == -1) {
		cerr << "I Did Not Find a Missing Link!" << endl;
		exit(-1);
	}
//error checking here VVVV
	info = Pm_GetDeviceInfo(id);
	cout << endl << "**opening input : " << info->name << endl << endl;
//error checking here VVVV
	Pm_OpenInput(&midi_in, id, NULL, 512, NULL, NULL);

	id = getMissingLink(false);
	if (id == -1) {
		cerr << "Could Not Find Missing Links Ouput" << endl;
		exit(-1);
	}
//error checking here VVVV
	info = Pm_GetDeviceInfo(id);
	cout << endl << "**opening output : " << info->name << endl << endl;
//error checking here VVVV
	Pm_OpenOutput(&midi_out, id, NULL, 0, NULL, NULL, 0);

	cout << "uploading file " << argv[1] << endl;
	//cout << "* make sure that you have put the microcontroller in boot loader mode" << endl;
	//cout << "hit enter to continue, or control-c to quit" << endl;
	//std::cin.get();

	//grab the page size!
	MLCommand(midi_in, midi_out, 1500, 3);
	pageSize = getPageSize(midi_in, midi_out, 3000);
	if (pageSize == 0) {
		cerr <<
		    "*** failed to get the page size from the microcontroller"
		    << endl;
		cerr << "*** are you in boot loader mode?" << endl;
		cerr <<
		    "*** do you have both the midi input and output to/from your microcontroller connected to your computer"
		    << endl;
		cerr << "*** did you select the correct input and output?" <<
		    endl;
		cerr << "*** aborting" << endl;
		exit(-1);
	} else if (pageSize != 128) {
		cerr << "*** something is amok !!! " << endl
		<< "    " << pageSize << " isnt the page size we are expecting (128)" << endl
		<< " Bailing ***" << endl;
		exit(-1);
	}


	//open the file
	inFile.open(argv[1], std::ios::in | std::ios::binary);
	if (!inFile.is_open()) {
		cerr << "cannot open file: " << argv[1] << endl;
		//Pm_Close(midi_out);
		Pm_Close(midi_in);
		exit(-1);
	}
	//read in the file data
	tmp = inFile.get();
	while (!inFile.eof()) {
		inputData.push_back(tmp);
		tmp = inFile.get();
	}
	//see if there is data
	if (inputData.size() < 1) {
		cerr << "datafile is empty" << endl;
		//Pm_Close(midi_out);
		Pm_Close(midi_in);
		exit(-1);
	}
	//compute the number of pages we'll need
	numPages = (unsigned short)ceil((float)inputData.size() / pageSize);
	if (numPages == 0)
		numPages = 1;
	//make sure there are enough positions in the pages
	pages.reserve(numPages);
	sysexMessages.reserve(numPages);

	//cut the data into pages
	for (unsigned short i = 0; i < inputData.size(); i++) {
		unsigned int pageIndex = i / pageSize;
		//if we're at a new page then push the page index, two bytes
		if (i % pageSize == 0) {
			pages.push_back(new std::vector < unsigned char >);
		}
		pages[pageIndex]->push_back(inputData[i]);
	}

	//pack and build the sysex messages
	cout << "byte packing the file contents" << endl;

	for (unsigned int i = 0; i < numPages; i++) {
		//create a new sysex message for each chunk
		for (unsigned int j = 0; j < pages[i]->size(); j += CHUNK_SIZE) {
			std::vector < unsigned char >tmp;
			unsigned int start_index = j;
			unsigned int end_index = j + CHUNK_SIZE;

			//make sure our index is in range
			if (end_index > pages[i]->size())
				end_index = pages[i]->size();
			std::vector < unsigned char >*newMsg =
			    new std::vector < unsigned char >;

			//write the header
			writeSysexPrefix(newMsg);
			newMsg->push_back(MIDIBOOT_FILLPAGE);

			//write address to tmp
			unsigned int pageAddress = i * pageSize + start_index;
			tmp.clear();
			tmp.push_back((unsigned char)(pageAddress >> 8) & 0xFF);
			tmp.push_back((unsigned char)pageAddress & 0xFF);

			//write the data to tmp
			for (unsigned int k = start_index; k < end_index; k++)
				tmp.push_back(pages[i]->at(k));

			//pack the address+data
			packedPage.clear();
			packedPage.resize(simple_bit_packed_length(tmp.size()));

			simple_bit_pack(&packedPage[0], &tmp[0], tmp.size());

			//write the packed address+data to the sysexMessage
			for (std::vector < unsigned char >::iterator it =
			     packedPage.begin(); it != packedPage.end(); it++)
				newMsg->push_back(*it);

			//write the sysex end message
			newMsg->push_back(MIDI_SYSEX_STOP);
			//push this sysex message onto our list of sysex messages
			sysexMessages.push_back(newMsg);
		}
		//create the 'write page' sysex message
		std::vector < unsigned char >*newMsg =
		    new std::vector < unsigned char >;
		writeSysexPrefix(newMsg);
		newMsg->push_back(MIDIBOOT_WRITEPAGE);
		newMsg->push_back(MIDI_SYSEX_STOP);
		//push this sysex message onto our list of sysex messages
		sysexMessages.push_back(newMsg);
	}

	cout << "page size: " << pageSize << endl;
	cout << "this many pages will be written: " << pages.size() << endl;
	cout << "this many bytes will be written: " << inputData.size() << endl;
	cout << "using this many sysex messages: " << sysexMessages.
	    size() << endl;

	//upload
	cout << "uploading now" << endl;

	//send each sysex message
	for (std::vector < std::vector < unsigned char >*>::iterator it =
	     sysexMessages.begin(); it != sysexMessages.end(); it++) {
		PmError error;
		unsigned char msg[64];

		//make sure we've only got zeros
		memset(&msg, 0, 64);

		//fill in sysex buffer
		for (unsigned int i = 0; i < (*it)->size(); i++) {
			msg[i] = (*it)->at(i);
		}

		//send the message
		error = Pm_WriteSysEx(midi_out, 0, msg);
		if (error) {
			cerr << "error: " << Pm_GetErrorText(error) << endl;
			exit(-1);
		}
		//get ack
		if (!waitForAck(midi_in, TIMEOUT_MS)) {
			cerr <<
			    "FAILED: sysex message failed to acknowledge within the timeout"
			    << endl;
			cerr << "was the microcontroller in boot loader mode?"
			    << endl;
			exit(-1);
		}
	}

	cout << "success" << endl;

	//leave boot
	unsigned char msg[64];
	memset(&msg, 0, 64);
	msg[0] = MIDI_SYSEX_START;
	for (unsigned int i = 0; i < MIDIBOOT_SYSEX_ID_LEN; i++)
		msg[i + 1] = midiboot_sysex_id[i];
	msg[MIDIBOOT_SYSEX_ID_LEN + 1] = MIDIBOOT_LEAVE_BOOT;
	msg[MIDIBOOT_SYSEX_ID_LEN + 2] = MIDI_SYSEX_STOP;
	//send the message
	PmError error = Pm_WriteSysEx(midi_out, 0, msg);
	if (error) {
		cerr << "error writing LEAVE BOOT msg: " <<
		    Pm_GetErrorText(error) << endl;
		exit(-1);
	}
	MLCommand(midi_in, midi_out, 1500, 1);

	Pm_Close(midi_out);
	Pm_Close(midi_in);
	return 0;
}
