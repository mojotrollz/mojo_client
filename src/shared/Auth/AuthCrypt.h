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

#ifndef _AUTHCRYPT_H
#define _AUTHCRYPT_H

#include <common.h>
#include "SARC4.h"

class BigNumber;

class AuthCrypt
{
    public:
        AuthCrypt();
        ~AuthCrypt();

        //Dummy
        void DecryptRecvDummy(uint8 *, size_t){return;};
        void EncryptSendDummy(uint8 *, size_t){return;};

        //3.3.5
        void Init_12340(BigNumber *K);
        void DecryptRecv_12340(uint8 *, size_t);
        void EncryptSend_12340(uint8 *, size_t);

        //1.12.X
        void Init_6005(BigNumber *K);
        void SetKey_6005(uint8 *, size_t);
        void DecryptRecv_6005(uint8 *, size_t);
        void EncryptSend_6005(uint8 *, size_t);

        bool IsInitialized() { return _initialized; }

        const static size_t CRYPTED_SEND_LEN_6005 = 6;
        const static size_t CRYPTED_RECV_LEN_6005 = 4;


    private:
        bool _initialized;
        //3.3.5
        SARC4 _decrypt;
        SARC4 _encrypt;
        
        //1.12.2
        std::vector<uint8> _key;
        uint8 _send_i, _send_j, _recv_i, _recv_j;
};
#endif
