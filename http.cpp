//============================================================================
// Name        : gethttp.cpp
// Author      : Bussysteme
// Version     : WiSe 09/10
//============================================================================


// Abfrage einer HTTP-URL und Ausgabe der Ergebnisse auf dem Bildschirm.
// F�r MS-Compiler: Die Linker-Datei wsock32.lib muss in den Projekteinstellungen
// eingetragen werden!

// update WS09/10: Dieses Beispiel l�uft auf Win und Unix.

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#else // Hier folgen die Ersetzungen f�r die BSD-socks.
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define  SOCKADDR_IN struct sockaddr_in
#define  SOCKADDR    struct sockaddr
#define  HOSTENT     struct hostent
#define  SOCKET      int
int WSAGetLastError() { return errno; }
int closesocket(int s) { return close(s); }
#endif
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")
#define _SIZE_OF_DATA 800000

void perr_exit(char* msg, int ret_code)
{
  printf("%s, Error: ",msg);
  printf("%d\n",ret_code);
  exit(ret_code);
}


int main(int argc, char** argv)
{
  char* site;
  char* host;
  char  url[1024];
  char  send_buf[256];
  char  rec_buf[256];
  char  rec_data[_SIZE_OF_DATA] = {0};
  long  rc;
  SOCKET s;
  SOCKADDR_IN addr;
#ifdef _WIN32
  WSADATA wsa;
#endif
  HOSTENT* hent;

  // Bevor man anfangen kann, muss man diese WSAStartup Funktion aufrufen.
  // Initialisiert TCPIP-Stack.
#ifdef _WIN32
  if(WSAStartup(MAKEWORD(2,0),&wsa))
    perr_exit("WSAStartup failed",WSAGetLastError());
#endif

  addr.sin_family=AF_INET;
  addr.sin_port=htons(80);

  printf("\nURL: ");
  scanf("%s",url);

  if(strncmp("http://",url,7)==0)
    host=url+7;
  else
    host=url;

  if((site=strchr(host,'/'))!=0)
    *site++='\0';
  else
    site=host+strlen(host); /* \0 */

  printf("Host: %s\n",host);
  printf("Site: %s\n",site);
  printf("Connecting....\n");

  if((addr.sin_addr.s_addr=inet_addr((const char*)host))==INADDR_NONE)
  {
    if(!(hent=gethostbyname(host)))
      perr_exit("Cannot resolve Host",WSAGetLastError());

    strncpy((char*)&addr.sin_addr.s_addr,hent->h_addr,4);

    if(addr.sin_addr.s_addr==INADDR_NONE)
      perr_exit("Cannot resolve Host",WSAGetLastError());
  }

  s=socket(AF_INET,SOCK_STREAM,0);
#ifdef _WIN32
  if (s==INVALID_SOCKET)
#else
  if (s<0)
#endif
    perr_exit("Cannot create Socket",WSAGetLastError());

  if( connect(s,(SOCKADDR*)&addr,sizeof(SOCKADDR)))
    perr_exit("Cannot connect",WSAGetLastError());

  printf("Connected to %s...\n",host);

  sprintf(send_buf, "GET /%s HTTP/1.0\nUser-Agent: xxxyyy\n\n", site);

  printf("Command sent to server: \n%s\n",send_buf);

  if((send(s,send_buf,strlen(send_buf),0))<strlen(send_buf))
    perr_exit("Cannot send Data",WSAGetLastError());

  //receive all data
  char* pointer = rec_data;
  char* upPointer = rec_data + _SIZE_OF_DATA;
  char* b_Pointer;
  while ((rc = recv(s, rec_buf, 256, 0)) > 0)
  {
      memcpy(pointer, rec_buf, rc);
      pointer = pointer + rc;           
      if (pointer >= upPointer)
      {
          printf("\nOverflow");
          exit(1);
      }
  }

  //Find the blank line
  b_Pointer = strstr(rec_data, "\r\n\r\n");
  b_Pointer += 4;  // get the body

  FILE* fp = fopen("output", "wb"); // write to the file
  if (!fp)
  {
      printf("\nCan not open the file");
      exit(1);
  }
  fwrite(b_Pointer, sizeof(char), strlen(b_Pointer), fp);
  fclose(fp);

  printf("\nDONE!");
  
  closesocket(s);
  return 0;
}
