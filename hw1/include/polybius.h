#ifndef POLYBIUS_H
#define POLYBIUS_H

void run_polybius(unsigned short mode);
void fillTable(int keyExists);
void encrypt();
int checkExists(char c);
void writeAddress(int checkExists, int col);
void encrypt(int col);
void decrypt(int col);
void printChar(int myRow, int myCol, int col);
#endif

