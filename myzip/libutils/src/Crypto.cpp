#include <string.h>
#include <string>
#include <memory>
#include <cstdlib>
#include <stdio.h>
#include "../include/CryptoDes.h"

#define DES3_INSERT_OTHER_MODE 0


#define SIMPLE_ENCRYPTO 1 //为了效率采用简单异或加密

#if SIMPLE_ENCRYPTO
static const char *pSimpleKey= "Auto-generated destructor stub";
#endif

CryptoDes::CryptoDes() {
	// TODO Auto-generated constructor stub

}

CryptoDes::~CryptoDes() {
	// TODO Auto-generated destructor stub
}

void CryptoDes::getKey(const std::string &key, unsigned char Des3key[DES3_KEY_LEN],int encryptMode) {
	size_t  len;
	memset((void*)Des3key,0,DES3_KEY_LEN);
	size_t maxlen=DES3_KEY_LEN;
	len=key.size()>maxlen ? maxlen :key.size();
	memcpy(Des3key,key.c_str(),key.size());

}

int CryptoDes::encrypt(const unsigned char *In, unsigned char *Out,
		unsigned long datalen, const char *Key, char bInsertType,int encryptMode) {

#if SIMPLE_ENCRYPTO
			printf("使用简易加密模式\n");
			int len= strlen(pSimpleKey);
			//memcpy(Out,In,datalen);
			for(int i=0;i<datalen;i++){
				Out[i] = In[i] ^ pSimpleKey[i%len];
			}
#else

			des3_context ctx3;
			unsigned char DES3_keys[DES3_KEY_LEN];
			unsigned char bInsertByte[8];
			unsigned char bFillByte;
			int i, j;
			const std::string key(Key);
			getKey(key, DES3_keys,encryptMode);
			des3_set_3keys(&ctx3, DES3_keys, DES3_keys + 8, DES3_keys + 16);
			j = datalen >> 3;
			for (i = 0; i < j; i++) {
				des3_encrypt(&ctx3, In, Out);
				Out += 8;
				In += 8;
			}
			if ((datalen & 7) != 0) {
				j = (datalen & 7);
				for (i = 0; i < j; i++)
					bInsertByte[i] = *(In + i);

				bFillByte = 0;
				if (bInsertType == DES3_INSERT_OTHER_MODE)
					bFillByte = 8 - j;

				for (i = j; i < 8; i++)
					bInsertByte[i] = bFillByte;

				des3_encrypt(&ctx3, bInsertByte, Out);
				datalen = (datalen + 8) & 0xfffffff8;
			} else {
				for (i = 0; i < 8; i++)
					bInsertByte[i] = 8;

				des3_encrypt(&ctx3, bInsertByte, Out);
				datalen = datalen + 8;
			}
#endif
			return datalen;
}
int CryptoDes::decrypt(const unsigned char *In, unsigned char *Out,
		unsigned long datalen, const char *Key, int nKeyLen, char bInsertType,int encryptMode) {
#if SIMPLE_ENCRYPTO
			int len= strlen(pSimpleKey);
			//memcpy(Out,In,datalen);
			for(int i=0;i<datalen;i++){
				Out[i] = In[i] ^ pSimpleKey[i%len];
			}
#else
			des3_context ctx3;
			unsigned char DES3_keys[24];
			int i, j;
			char bLastByte;
			char bInsertByte[8];
			if ((datalen & 7) != 0)
				return 0;

			;
			const std::string key(Key,nKeyLen);
			getKey(key, DES3_keys,encryptMode);

			des3_set_3keys(&ctx3, DES3_keys, DES3_keys + 8, DES3_keys + 16);

			j = datalen >> 3;
			for (i = 0; i < j; i++) {
				des3_decrypt(&ctx3, In, Out);
				Out += 8;
				In += 8;
			}
			if (bInsertType == DES3_INSERT_OTHER_MODE) {
				bLastByte = *(Out - 1);
				// if(bLastByte <=7)
				{
					for (i = (8 - bLastByte); i < 8; i++)
						bInsertByte[i] = bLastByte;
					if (memcmp(&bInsertByte[8 - bLastByte], (Out - bLastByte),
						bLastByte) == 0){ 
							return (datalen - bLastByte);
					}
				}

			}  
#endif
			return datalen;
}
