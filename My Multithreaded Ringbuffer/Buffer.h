#pragma once

#include <atomic>
#include <stdint.h>
#include <mutex>
#include <iostream>


#define MAX_READERS_NUMBER 8

template<typename DataType, int numberOfSlots>
class MultipleWriterReaderBuffer{

public:

	std::mutex mtx;

	char buffer[sizeof(DataType) * numberOfSlots];
	char* bufferPointer;

	std::atomic<uint8_t> readWriteKeys[numberOfSlots];
	uint32_t readersSlotNumber[MAX_READERS_NUMBER]; //This is for each reader to know what slot to read in the buffer
	uint64_t writerTotalSlotsWritten = 0;
	uint8_t numberOfReadersRegistered = 0;
	uint8_t readersMaskForWriting = 0;

	MultipleWriterReaderBuffer(){

		memset(buffer, 0, sizeof(DataType) * numberOfSlots);
		bufferPointer = buffer;

		readWriteKeys[numberOfSlots] = { 0 };
		readersSlotNumber[MAX_READERS_NUMBER] = { 0 };

	}

	~MultipleWriterReaderBuffer(){

	}

	uint8_t registerReaderIndex(){

		if (numberOfReadersRegistered == MAX_READERS_NUMBER){
			std::cout << "Error! Maximum number of readers reached." << std::endl;
			return -1;
		}
		else{
			uint8_t thisIndex = numberOfReadersRegistered;
			numberOfReadersRegistered += 1;

			readersMaskForWriting = readersMaskForWriting << 1;
			readersMaskForWriting = readersMaskForWriting | 1;

			return thisIndex;
		}

	}

	void write(DataType &data){

		//To prevent race condition caused by duplicated writerSlotNumber
		mtx.lock();

		//Get the slot number to use in the buffer
		uint64_t writerSlotNumber = writerTotalSlotsWritten % numberOfSlots;

		//One more message has been written
		writerTotalSlotsWritten++;

		mtx.unlock();

		//Check if the writer has caught up with the last unread piece of data
		if (readWriteKeys[writerSlotNumber] > 0){
			std::cout << "Error! Tried to write into memory that has not been read by all readers." << std::endl; 
			return;
		}

		//Actual line of writing
		memcpy(bufferPointer + (sizeof(DataType) * writerSlotNumber), &data, sizeof(DataType));

		//Informing the readers that for this specific slot, there is data to be read
		readWriteKeys[writerSlotNumber] = 255 & readersMaskForWriting;

	}

	bool read(DataType* &data, int readerIndex){

		uint8_t mask = pow(2, readerIndex);
		uint8_t thisSlotNumber = readersSlotNumber[readerIndex];
		uint8_t thisSlotMask = readWriteKeys[thisSlotNumber];

		//There is data to be read
		uint8_t and = mask & thisSlotMask;
		if ((mask & thisSlotMask) == mask){

			data = (DataType*)(bufferPointer + sizeof(DataType) * thisSlotNumber);

			//Flip the bit from 1 to 0
			readWriteKeys[thisSlotNumber] = readWriteKeys[thisSlotNumber] ^ mask;

			readersSlotNumber[readerIndex] = (readersSlotNumber[readerIndex] + 1) % numberOfSlots;

			return true;
		}
		return false;

	}

};