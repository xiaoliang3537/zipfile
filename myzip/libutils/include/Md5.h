/*
 * MD5.h
 *
 *  Created on: Sep 1, 2012
 *      Author: jevey
 */

#ifndef MD5_H_
#define MD5_H_

#include <string>
#include <fstream>

using namespace std;
 

#define uint32 unsigned int


class MD5
{
public:
	MD5();
	MD5(const int source);
	MD5(const string& source);
	MD5(const unsigned char* source, uint32 len);

	string Calculate(const string& source);
	string Calculate(ifstream& file);
	string Calculate(const unsigned char* source, uint32 len);
    string Calculate16BitsMd5(const unsigned char* source, uint32 len);

	string GetHash() const;
	const unsigned char* GetRawHash() const { return m_rawHash; }

private:
	string	m_sHash;
	unsigned char m_rawHash[16];
};

#endif
 
