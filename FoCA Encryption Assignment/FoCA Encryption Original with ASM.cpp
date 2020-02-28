
char EKey = 'X';

#define StudentName "Andrei Stroe"

#define MAXCHARS 6

using namespace std;
#include <string>               // for strings
#include <fstream>              // file I/O
#include <iostream>             // for cin >> and cout <<
#include <iomanip>              // for fancy output
#include "TimeUtils.h"          // for GetTime, GetDate, etc.

#define dollarchar '$'          // string terminator
#define OChars      0           // 2D index of original character string
#define EChars      1           // 2D index of encrypted character string
#define DChars      2           // 2D index of decrypted character string

char EncryptionData[3][MAXCHARS];  // Multidimensional Array to store all data

//----------------------------- C++ Functions ----------------------------------------------------------

void get_char(char& a_character)
{
  a_character=(char) _getwche();


  __asm {
	  mov eax, a_character
	  movzx eax, byte ptr[eax]

	  wloop:
	  cmp eax, 0x0D       // the compares the char to the end of a line (enter)
	  je endloop          // it then jumps to the terminate the function if it is an end of line
	  cmp eax, 0x24       // if not it compares the char to the dollarchar ($)
	  je endloop          // if it is a dollarchar it again terminates the function
	  cmp eax, '0'        // this compares the char to '0'
	  jl fail             // if it is less than zero it will jump to the fail part and prompt the user to to try again 
	  cmp eax, 'z'        // this compares the char to 'z'
	  jg fail             // if it is bigger than 'z' it will jump to the fail part and prompt the user to to try again 
	  jmp endloop         // if it passes everything, it will jump to the end of the loop succesfully.
  }
  

  __asm {fail:}
cout << "\nAlphanumeric characters only, please try again > ";
a_character = (char)_getwche();


__asm {
	jmp wloop
}

__asm {endloop:}
  if (a_character == '\r' || a_character == '\n')
  {
      a_character = dollarchar;
  }

}
//-------------------------------------------------------------------------------------------------------------

void get_original_chars(int& length)
{  
  char next_char = ' ';
  length = 0;
  get_char(next_char);

  while ((length < MAXCHARS) && (next_char != dollarchar))
  {
    EncryptionData[OChars][length++] = next_char;
    get_char(next_char);
  }
}

//---------------------------------------------------------------------------------------------------------------
//----------------- ENCRYPTION ROUTINES -------------------------------------------------------------------------

void encrypt_chars (int len, char EKey)
{   
  char temp_char;                       // Character temporary store
  int i;                                
  __asm {
                                        // the forloop in assembly 
   mov i, 0                             // i = 0
   jmp checkend                         // unconditional jump to check if the forloop can end

   startfor : mov EBX, i                // Copies 'i' into ebx register
              add EBX, 1                
              mov i, EBX
              cmp len, EBX              
              jl endloop               //if EBX is greater than the length of the array it jumps to the end of the loop
  
   checkend : mov EBX, i
              mov AL, byte ptr[EncryptionData + OChars + ebx]  // moves EncryptionData[OChars][i] into the AL register
              mov temp_char, AL
  }
                                   
__asm 
  {                                    
   push   EAX                   // pushes the eax register to the top of the call stack.
   push   ECX                   // pushes ecx to the top of the call stack.
   push   EDX						        // pushes edx to the top of the call stack.

   lea      EAX, [temp_char]    // this loads the address (of 3-bits after ebp in the stack), into eax. 
   movzx    ECX, [EKey]			    // this moves ecx to ebp+8 (2-bits after ebp in the stack)

     
   cmp temp_char, 'A'             //
   jl endloop1                   //
   cmp temp_char, 'Z'           //    Here i am checking if the current char is between A - Z
   jg endloop1                 //
   add temp_char, 0x20        // if it is between A - Z it will add 0x20 (32) which will equal the lowercase equivalent

   endloop1:
   push     ECX					           // This pushes the ecx register to the top of the call stack. This is the character that is to be decrypted.
   not byte ptr[EAX]			         // This inverts the eax register, which is the 32-bit address of my Ekey 'x'.
   add      byte ptr[EAX], 0x04	   // This adds 4 bytes to the eax register which moves it up 1 place on the call stack.
   movzx    EDI, byte ptr[EAX]	   // This moves the value of the eax register into the edi register, so edi now holds the Ekey.
   pop      EDX					           // the edx register now holds the value of of the charchter that needs enycrypting.
   xor      EDX, EDI			         // this compares the Ekey (edi) to the edx register which is the encrypted value of the original character.
   rol      DL, 2   				       // this rotates the dl register left by 1 bit, which essentially doubles the value.
   add      DL, 0x04				       // this adds 4 to the dl register, which moves it up one place in the call stack.
   mov		temp_char, DL

   pop    EDX                     // this stores the value of the top of the call stack into edx.
   pop    ECX                     // this stores the value of the top of the call stack into ecx.
   pop    EAX						          // this stores the value of the top of the call stack into eax.
  }
  __asm { 
      mov AL, temp_char
      mov byte ptr[EncryptionData + EBX + 6], AL   // 6 is used as it is the memory address of the column
  jmp startfor
  }
  __asm{endloop:}
  return;
}




//---------------------------------------------------------------------------------------------------------------
//----------------- DECRYPTION ROUTINES -------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
void decrypt_chars (int length, char EKey)
{
	char temp_char;                       // Character temporary store

	for (int i = 0; i < length; i++)      // Encrypt characters one at a time
	{
		temp_char = EncryptionData[EChars][i];

		__asm
		{
		push   EAX                         // pushes the eax register to the top of the call stack.
		push   ECX                         // pushes ecx to the top of the call stack.
		push   EDX					            	 // pushes edx to the top of the call stack.
										                   // the order pf the call stack is now edx, ecx and then eax.
		push   temp_char					         // this takes whatever char the loop is on, and put it at the top of the call stack.
		push   EKey							           // this puts my EKey 'X' at the top of the call stack, above the char.

		call   decrypt_4                   // this runs the decryption funtion which encrypts one char at a time.
		mov    temp_char, dl               // this stores the decrypted char that was returned into the dl register.

		add    ESP, 8						           // this moves the stack pointer up by 2-bits.
		pop    EDX                         // this stores the value of the top of the call stack into edx.
		pop    ECX                         // this stores the value of the top of the call stack into ecx.
		pop    EAX						             // this stores the value of the top of the call stack into eax.

		}
  EncryptionData[DChars][i] = temp_char;
	}
  return;

  __asm
  {
  decrypt_4:

	push EBP				                      // this saves the value for the calling function
	mov EBP, ESP			                    // this copies esp (the stack pointer) into ebp, so that ebp points to the function's call stack
	lea    EAX, [EBP + 12]	              // this loads the address (of 3-bits after ebp in the stack), into eax. 
	movzx  ECX, [EBP + 8]	                // this moves ebp+8 (2-bits after ebp in the stack) into the entire register of ecx.
                                           
	push  EDI				                      // this pushes the edi register to the top of the call stack.
	push  ECX				                      // This puts the ecx register to the top of the call stack, one above edi.
	                                       
                                       
	sub   byte ptr[EAX], 0x04	            // this subtracts 4 from the eax register, which moves it down one place in the call stack.
	ror   byte ptr[EAX], 1		            // this rotates the eax register right by 1 bit, which essentially halves the value.
	ror   byte ptr[EAX], 1		            // again this halves the value of the eax register.
	                                     
	movzx EDX, byte ptr[EAX]              // this moves the byte pointer of eax into the edx register
	                                     
	pop EDI                               // this puts whatever is at the top of the call stack into edi.
	xor EDX, EDI				                  // this compares the Ekey (edi) to the edx register which is the decrypted value.
                                       
                                       
	sub  EDX, 0x04                        // This subtracts 4 bytes from the edx register which moves it down 1 place on the call stack.
	neg  EDX				                      // This negates the edx register, which is the 32-bit address of my Ekey 'x'.
	sub  EDX, 1                           // this subtracts 1 from the decrypted charchter which makes the same as the original charchter
	pop  EDI                              // this restores the edi register.
	                                     
	pop EBP					                      // this restores the ebp register.
	ret							                      // before the funtion ends, it returns the decrypted character. 
  }
}
//*** end of decrypt_chars function
//---------------------------------------------------------------------------------------------------------------



int main(void)
{
  int char_count (0);  // The number of actual characters entered (upto MAXCHARS limit).
  
  cout << "\nPlease enter upto " << MAXCHARS << " alphanumeric characters:  ";
  get_original_chars (char_count);

  ofstream EDump;
  EDump.open("EncryptDump.txt", ios::app);
  EDump << "\n\nFoCA Encryption program results (" << StudentName << ") Encryption key = '" << EKey << "'";
  EDump << "\nDate: " << GetDate() << "  Time: " << GetTime();
    
  // Display and save initial string
  cout << "\n\nOriginal string =  " << EncryptionData[OChars] << "\tHex = ";
  EDump<< "\n\nOriginal string =  " << EncryptionData[OChars] << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << hex << setw(2) << setfill('0') << ((int(EncryptionData[OChars][i])) & 0xFF) << "  ";
    EDump<< hex << setw(2) << setfill('0') << ((int(EncryptionData[OChars][i])) & 0xFF) << "  ";
  };

  //*****************************************************
  // Encrypt the string and display/save the result
  encrypt_chars (char_count, EKey);

  cout << "\n\nEncrypted string = " << EncryptionData[EChars] << "\tHex = ";
  EDump<< "\n\nEncrypted string = " << EncryptionData[EChars] << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << ((int(EncryptionData[EChars][i])) & 0xFF) << "  ";
    EDump<< ((int(EncryptionData[EChars][i])) & 0xFF) << "  ";
  }

  //*****************************************************
  // Decrypt the encrypted string and display/save the result
  decrypt_chars (char_count, EKey);

  cout << "\n\nDecrypted string = " << EncryptionData[DChars] << "\tHex = ";
  EDump<< "\n\nDecrypted string = " << EncryptionData[DChars] << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << ((int(EncryptionData[DChars][i])) & 0xFF) << "  ";
    EDump<< ((int(EncryptionData[DChars][i])) & 0xFF) << "  ";
  }
  //*****************************************************

  cout << "\n\n\n";
  EDump << "\n\n-------------------------------------------------------------";
  EDump.close();
  
  system("PAUSE");
  return (0);
}


