#include <iostream>
#include <thread>
#include "Buffer.h"

MultipleWriterReaderBuffer<int, 256> buffer;

void writer(){

	int a = 3;
	int b = 24;
	buffer.write(a);
	buffer.write(b);

}

void reader(int index){

	while (true){

		int* space;

		if (buffer.read(space, index))
			std::cout << *space << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	}

}

int main(){

	int readerIndex0 = buffer.registerReaderIndex();
	int readerIndex1 = buffer.registerReaderIndex();

	std::thread writerThread0(writer);
	std::thread writerThread1(writer);
	std::thread readerThread0(reader, readerIndex0);
	std::thread readerThread1(reader, readerIndex1);
	
	while (true){}

}