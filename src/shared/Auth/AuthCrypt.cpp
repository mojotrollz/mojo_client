/*
 * Copyright (C) 2005-2009 MaNGOS <http://mangosclient.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "AuthCrypt.h"
#include "Hmac.h"
#include "BigNumber.h"

AuthCrypt::AuthCrypt()
{
    _initialized = false;
}

AuthCrypt::~AuthCrypt()
{

}

// 3.3.5
void AuthCrypt::Init_12340(BigNumber *K)
{
    uint8 ClientDecryptionKey[SEED_KEY_SIZE] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };
    HmacHash serverEncryptHmac(SEED_KEY_SIZE, (uint8*)ClientDecryptionKey);
    uint8 *decryptHash = serverEncryptHmac.ComputeHash(K);

    uint8 ServerEncryptionKey[SEED_KEY_SIZE] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };
    HmacHash clientDecryptHmac(SEED_KEY_SIZE, (uint8*)ServerEncryptionKey);
    uint8 *encryptHash = clientDecryptHmac.ComputeHash(K);

    //SARC4 _serverDecrypt(encryptHash);
    _decrypt.Init(decryptHash);
    _encrypt.Init(encryptHash);
    //SARC4 _clientEncrypt(decryptHash);

    uint8 syncBuf[1024];

    memset(syncBuf, 0, 1024);

    _encrypt.UpdateData(1024, syncBuf);
    //_clientEncrypt.UpdateData(1024, syncBuf);

    memset(syncBuf, 0, 1024);

    //_serverDecrypt.UpdateData(1024, syncBuf);
    _decrypt.UpdateData(1024, syncBuf);

    _initialized = true;
}

void AuthCrypt::DecryptRecv_12340(uint8 *data, size_t len)
{
    if (!_initialized)
        return;

    _decrypt.UpdateData(len, data);
}

void AuthCrypt::EncryptSend_12340(uint8 *data, size_t len)
{
    if (!_initialized)
        return;

    _encrypt.UpdateData(len, data);
}


void AuthCrypt::Init_8606(BigNumber *K)
{
    uint8 *key = new uint8[SHA_DIGEST_LENGTH];

    uint8 recvSeed[SEED_KEY_SIZE] = { 0x38, 0xA7, 0x83, 0x15, 0xF8, 0x92, 0x25, 0x30, 0x71, 0x98, 0x67, 0xB1, 0x8C, 0x4, 0xE2, 0xAA };
    HmacHash recvHash(SEED_KEY_SIZE, (uint8*)recvSeed);
    recvHash.UpdateBigNumber(K);
    recvHash.Finalize();
    memcpy(key, recvHash.GetDigest(), SHA_DIGEST_LENGTH);

    _key.resize(SHA_DIGEST_LENGTH);
    std::copy(key, key + SHA_DIGEST_LENGTH, _key.begin());
    delete[] key;

    _send_i = _send_j = _recv_i = _recv_j = 0;
    _initialized = true;
}


//1.12.2
void AuthCrypt::Init_6005(BigNumber *K)
{
    _send_i = _send_j = _recv_i = _recv_j = 0;

    SetKey_6005(K->AsByteArray(),40);
    _initialized = true;

}

void AuthCrypt::DecryptRecv_6005(uint8 *data, size_t len)
{
    if (!_initialized)
        return;

    if (len < CRYPTED_RECV_LEN_6005)
        return;

    for (size_t t = 0; t < CRYPTED_RECV_LEN_6005; t++)
    {
        _recv_i %= _key.size();
        uint8 x = (data[t] - _recv_j) ^ _key[_recv_i];
        ++_recv_i;
        _recv_j = data[t];
        data[t] = x;


    }

}

void AuthCrypt::EncryptSend_6005(uint8 *data, size_t len)
{
    if (!_initialized)
        return;
    if (len < CRYPTED_SEND_LEN_6005)
        return;

    for (size_t t = 0; t < CRYPTED_SEND_LEN_6005; t++)
    {
        _send_i %= _key.size();
        uint8 x = (data[t] ^ _key[_send_i]) + _send_j;
        ++_send_i;
        data[t] = _send_j = x;
    }

}

void AuthCrypt::SetKey_6005(uint8 *key, size_t len)
{
    _key.resize(len);
    std::copy(key, key + len, _key.begin());
}
