#ifndef CRYPTODES_H_
#define CRYPTODES_H_

#include "3Des.h"

#define DES3_KEY_LEN 24
//#define DES3_KEY "LOVEIJMTEST.."
//static const char DES3_KEY[]= {0x20,0x01,0x02,0x08,0x88,0x10,0x79,0x58,0x26,0x32,0x08,0x04,0x01,0x00};

#define CRYPTO_KEY 1

class CryptoDes {
public:
	CryptoDes();
	virtual ~CryptoDes();
public:
	static void getKey(const std::string &key, unsigned char Des3key[DES3_KEY_LEN],int encryptMode);
	static int encrypt(const unsigned char *In, unsigned char *Out,
			unsigned long datalen, const  char *Key, char bInsertType,int encryptMode);
	static int decrypt(const unsigned char *In, unsigned char *Out,
			unsigned long datalen, const char *Key, int nKeyLen, char bInsertType,int encryptMode);


};

#endif
