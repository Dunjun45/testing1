#include <fstream>
#include <iostream>
#include <vector>

#define CRC32_POLYNOMIAL 0xEDB88320

unsigned long CRC32Value(int i) {
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--) {
		if (ulCRC & 1) {
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		}
		else {
			ulCRC >>= 1;
		}
	}
	return ulCRC;
}

unsigned long ComputeBlockCRC32(unsigned long length, unsigned char* buffer) {
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (length-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC^*buffer++) & 0xff);
		ulCRC = ulTemp1^ulTemp2;
	}
	return(ulCRC);
}

unsigned long readCRC(char* bufptr) {
	unsigned long crc = (unsigned long)bufptr[0] << 24 |
		(unsigned long)bufptr[1] << 16 |
		(unsigned long)bufptr[2] << 8 |
		(unsigned long)bufptr[3];
	return crc;
}

void writeInt(char* bufptr, int value) {
	bufptr[0] = (value >> 24) & 0xFF;
	bufptr[1] = (value >> 16) & 0xFF;
	bufptr[2] = (value >> 8) & 0xFF;
	bufptr[3] = (value) & 0xFF;
}

struct highscore {
	char name[16];
	int score;

	highscore(const char* n, int s) {
		memset(name, 0, 16);
		strcpy(name, n);
		score = s;
	}

	highscore(char* buf) {
		memset(name, 0, 16);
		strcpy(name, buf);
		score = readCRC(buf + 16);
	}
};

void writeHighScoreToFile(std::vector<highscore>& highscores, std::string outputFilename) {
	std::ofstream out(outputFilename, std::ofstream::binary);
	unsigned long buflen = (highscores.size() * 20) + 4;
	char* buffer = new char[buflen];
	char* bufptr = buffer;
	for (int i = 0; i < highscores.size(); ++i) {
		char* nameptr = highscores[i].name;
		int score = highscores[i].score;
		memcpy(bufptr, nameptr, 16);
		bufptr += 16;
		writeInt(bufptr, score);
		bufptr += 4;
	}
	unsigned long crc = ComputeBlockCRC32(buflen - 4, reinterpret_cast<unsigned char*>(buffer));
	std::cout << "CRC: " << crc << std::endl;
	writeInt(bufptr, crc);
	out.write(buffer, buflen);
	delete[] buffer;
}

bool readHigScoresFromFile(std::vector<highscore>& highscores, std::string inputFilename) {
	std::ifstream in(inputFilename, std::ifstream::binary);
	in.seekg(0, in.end);
	int length = in.tellg();
	in.seekg(0, in.beg);
	char* buffer = new char[length];
	in.read(buffer, length);
	unsigned long crc = readCRC(buffer + length - 4);
	std::cout << "CRC Read: " << crc << std::endl;
	unsigned long computedcrc = ComputeBlockCRC32(length - 4, reinterpret_cast<unsigned char*>(buffer));
	std::cout << "CRC computed: " << computedcrc << std::endl;
	int num_scores = (length - 4) / 20;
	std::cout << "Num Scores: cool " << num_scores << std::endl;
	char* bufptr = buffer;
	for (int i = 0; i < num_scores; i++) {
		highscore h(bufptr);
		bufptr += 20;
		highscores.push_back(h);
	}
	return computedcrc == crc;
}

int main() {

	std::vector<highscore> scores{ highscore("abc", 45),highscore("jaja", 34), highscore("yiyi", 12) };
	writeHighScoreToFile(scores, "test.txt");
	scores.clear();
	readHigScoresFromFile(scores, "test.txt");
	for (highscore& h : scores) {
		std::cout << "name: " << h.name << std::endl;
		std::cout << "score: " << h.score << std::endl;
	}
	system("Pause");
}