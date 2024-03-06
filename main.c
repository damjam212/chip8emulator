#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <signal.h>

// Definicje
#define MEMORY_SIZE 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

FILE *file;
//uint8_t unsingend 8bit int
typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[16];  // Rejestry ogólnego przeznaczenia V0-VF
    uint16_t I;     // Rejestr indeksowy
    uint16_t pc;    // Licznik programu
    uint16_t stack[16];
    uint8_t sp;     // Wskaźnik stosu
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t drawFlag;
    uint8_t keys[16];
    uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT]; // Bufor ekranu
} Chip8;

//http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.1 link z instrukcjami
char mapButtons[] = {'X', '1', '2', '3', 'Q', 'W', 'E', 'A', 'S', 'D', 'Z', 'C', '4', 'R', 'F', 'V'};
// Inicjalizacja emulatora
void initializeChip8(Chip8 *chip8) {
    // Zerowanie pamięci
    memset(chip8->memory, 0, sizeof(chip8->memory));

    // Zerowanie rejestrów V0-VF, indeksowego, licznika programu, wskaźnika stosu
    memset(chip8->V, 0, sizeof(chip8->V));
    chip8->I = 0;
    chip8->pc = 0x200; // Adres początkowy programu w pamięci
    memset(chip8->stack, 0, sizeof(chip8->stack));
    chip8->sp = 0;

    // Zerowanie liczników opóźnienia i dźwięku
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->drawFlag = 0;

    // Zerowanie klawiszy
    memset(chip8->keys, 0, sizeof(chip8->keys));

    // Wyczyszczenie bufora ekranu
    memset(chip8->screen, 0, sizeof(chip8->screen));

    // Inicjalizacja pamięci z zestawem znaków (do wyświetlania)
    // ...

    // Wczytanie programu do pamięci (tutaj można dodać opcję wczytania pliku jako argumentu)
    // ...

    // Inne inicjalizacje, jeśli potrzebne
    // ...
}
void handle_sigint(int sig) {
    printf("\nOtrzymano sygnał SIGINT (Ctrl+C)\n");
        fclose(file);
    exit(0);
}


// Wczytanie programu do pamięci
bool loadProgram(Chip8 *chip8, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return false; // Błąd przy otwieraniu pliku
    }

    // Określ długość pliku
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // nie zmiesci sie w pamieci
    // adresacja zaczyna sie od 0x200(215) do 4095
    if (file_size > MEMORY_SIZE - 0x200) {
        fclose(file);
        return false; // Plik jest zbyt duży, aby wczytać go do pamięci
    }

    // Odczytaj zawartość pliku i zapisz ją w pamięci, począwszy od adresu 0x200
    fread(&chip8->memory[0x200], file_size, 1, file);

    fclose(file);
    return true;
}


void updateInput(Chip8 *chip8) {
        //    printf('input updarte');
    // for (int i = 0; i < 16; i++) {
    //     chip8->keys[i] = 0;
    // }
    for (int i = 0; i < 16; i++) 
    {
    if (GetAsyncKeyState(mapButtons[i]))
	{
		chip8->keys[i] = 1;
	}else
	{
     chip8->keys[i] = 0;
	}	
    }

    // int key = getchar(); // Odczytaj klawisz

    // if (key >= '0' && key <= '9') 
    // {
    //  //   printf('Nacisnales %c',key);
    //     chip8->keys[key - '0'] = 1; // Ustaw odpowiednią pozycję w tablicy kluczy
    // } else if (key >= 'a' && key <= 'f') {
    //   //  printf('Nacisnales %c',key);
    //     chip8->keys[key - 'a' + 10] = 1; // Ustaw odpowiednią pozycję w tablicy kluczy
    // }
}

void updateOutput(Chip8 *chip8, HANDLE h)
{

   // system("clear");
    if(chip8->drawFlag==1)
    {
    COORD begin;
	begin.X = 0;
	begin.Y = 0;
	SetConsoleCursorPosition(h, begin);
   // system("clear");
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (chip8->screen[x + y * SCREEN_WIDTH] == 1) 
            {
                printf("*"); // Wypisanie piksela
            } else {
                printf(" "); // Wypisanie pustego miejsca
            }
        }
        printf("\n"); // Przejście do nowego wiersza
    }
    chip8->drawFlag==0;
    }
    else
    {

    }
}
// lista insturkcji
/*
            00E0 - CLS
            00EE - RET
            0nnn - SYS addr
            1nnn - JP addr
            2nnn - CALL addr
            3xkk - SE Vx, byte
            4xkk - SNE Vx, byte
            5xy0 - SE Vx, Vy
            6xkk - LD Vx, byte
            7xkk - ADD Vx, byte
            8xy0 - LD Vx, Vy
            8xy1 - OR Vx, Vy
            8xy2 - AND Vx, Vy
            8xy3 - XOR Vx, Vy
            8xy4 - ADD Vx, Vy
            8xy5 - SUB Vx, Vy
            8xy6 - SHR Vx {, Vy}
            8xy7 - SUBN Vx, Vy
            8xyE - SHL Vx {, Vy}
            9xy0 - SNE Vx, Vy
            Annn - LD I, addr
            Bnnn - JP V0, addr
            Cxkk - RND Vx, byte
            Dxyn - DRW Vx, Vy, nibble
            Ex9E - SKP Vx
            ExA1 - SKNP Vx
            Fx07 - LD Vx, DT
            Fx0A - LD Vx, K
            Fx15 - LD DT, Vx
            Fx18 - LD ST, Vx
            Fx1E - ADD I, Vx
            Fx29 - LD F, Vx
            Fx33 - LD B, Vx
            Fx55 - LD [I], Vx
            Fx65 - LD Vx, [I]*/



// chip8 jedna instrukcja jest 2-bajtowa

//--CYKL PROCESORA--
//
// cykl procesora
// odczyt instrukcji
// dekodowanie instrukcji i ustalenie jej parametrów
// wykonanie instrukcji
// aktualizacja stanu procesora
// obsułga przerwań
// Emulacja pojedynczego cyklu
void emulateCycle(Chip8 *chip8,HANDLE h) 
{
  
   // fprintf(stderr,"executing");

    updateInput(chip8);
    // Odczytaj i zdekoduj instrukcję
    // Wykonaj odpowiednią operację zgodnie z instrukcją
    // Zaktualizuj rejestry, pamięć, opóźnienie, dzwięk itp.

    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
   // fprintf(stderr, "Wartość uint16_t: %u\n", opcode);
      //   fprintf(stderr,opcode);
    // Zdekodowanie i wykonanie instrukcji
    fprintf(file, "%u\n", opcode);
    fprintf(file, "%u\n", chip8->pc);

switch (opcode & 0xF000) 
{
    uint8_t x;
    uint8_t y;
    uint8_t kk;
    uint16_t nnn;
    uint8_t n;

    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    kk = opcode & 0x00FF;     
    nnn = (opcode & 0x0FFF) ; 
    n = (opcode & 0x000F);// Pobranie n
    case 0x0000:
        switch (opcode & 0x00FF) 
        {
            case 0x00E0:
                // Obsługa instrukcji 00E0 (wyczyść ekran)
                memset(chip8->screen, 0, sizeof(chip8->screen));
                chip8->pc += 2;
                chip8->drawFlag = 1;
                break;

            case 0x00EE:
                // Obsługa instrukcji 00EE (powrót z podprogramu)
                chip8->sp--;
                chip8->pc = chip8->stack[chip8->sp];
                chip8->pc += 2;
                break;

        }
        break;

    case 0x1000:
        // Skok do adresu
        //0x1nnn
        chip8->pc = opcode & 0x0FFF;
    break;

    case 0x2000:
        // Wywołanie podprogramu
         /*
         2nnn - CALL addr
            Call subroutine at nnn.

            The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.*/
        //
        chip8->stack[chip8->sp] = chip8->pc;
        chip8->sp++;
        chip8->pc = opcode & 0x0FFF;
    break;

    /*
    3xkk - SE Vx, byte
    Skip next instruction if Vx = kk.

    The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.*/
    case 0x3000:
         x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
         kk = opcode & 0x00FF;       // Pobranie wartości kk
         if (chip8->V[x] == kk) 
         {
         chip8->pc += 4; // Skok do następnej instrukcji
         }
          else 
          {
         chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
          }
    
    break;

    case 0x4000:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
        kk = opcode & 0x00FF;       // Pobranie wartości kk
         if (chip8->V[x] != kk) 
         {
         chip8->pc += 4; // Skok do następnej instrukcji
         }
          else 
          {
         chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
          }
    
    break;

    //5xy0 - SE Vx, Vy

    case 0x5000:
         x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
         y = (opcode & 0x00F0) >> 4;// Pobranie numeru rejestru Vy
         if (chip8->V[x] == chip8->V[y]) 
         {
         chip8->pc += 4; // Skok do następnej instrukcji
         }
          else
          { 
         chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
          }
    
    break;
  /*
    6xkk - LD Vx, byte
    Set Vx = kk.

    The interpreter puts the value kk into register Vx.
*/

    case 0x6000:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
        kk = opcode & 0x00FF;  
        chip8->V[x] = kk;
        chip8->pc += 2;

    
    break;

    /*
    7xkk - ADD Vx, byte
Set Vx = Vx + kk.

Adds the value kk to the value of register Vx, then stores the result in Vx.

    */
    case 0x7000:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
        kk = opcode & 0x00FF;  
        chip8->V[x] += kk;
        chip8->pc += 2;

    
    break;
    // Inne przypadki dla różnych instrukcji
    // ...

   case 0x8000:
   switch(opcode & 0x000F)
   {
    case 0x0000:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
        y = (opcode & 0x00F0) >> 4;// Pobranie numeru rejestru Vy
        chip8->V[x] = chip8->V[y];

        chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
    break;
    case 0x0001:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
        y = (opcode & 0x00F0) >> 4;// Pobranie numeru rejestru Vy
        chip8->V[x] =  chip8->V[x] | chip8->V[y];

        chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
    break;
    case 0x0002:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
        y = (opcode & 0x00F0) >> 4;// Pobranie numeru rejestru Vy
        chip8->V[x] =  chip8->V[x] & chip8->V[y];

        chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
    break;
    case 0x0003:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
        y = (opcode & 0x00F0) >> 4;// Pobranie numeru rejestru Vy
        chip8->V[x] =  chip8->V[x] ^ chip8->V[y];

        chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
    break;
    case 0x0004:
    x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
    y = (opcode & 0x00F0) >> 4; // Pobranie numeru rejestru Vy

    if (chip8->V[x]+ - chip8->V[y] > 0xFF) {
        chip8->V[0xF] = 1; // Ustawienie flagi przeniesienia (carry) na 1
    } else {
        chip8->V[0xF] = 0; // Ustawienie flagi przeniesienia (carry) na 0
    }

    chip8->V[x] += chip8->V[y]; // Dodanie wartości rejestru Vy do Vx
    chip8->pc += 2; // Normalne przesunięcie na następną instrukcję

    break;

    case 0x0005:
    x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
    y = (opcode & 0x00F0) >> 4; // Pobranie numeru rejestru Vy

    if (chip8->V[x] > chip8->V[y]) 
    {
        chip8->V[0xF] = 1; // Ustawienie flagi przeniesienia (borrow) na 1
    } else {
        chip8->V[0xF] = 0; // Ustawienie flagi przeniesienia (borrow) na 0
    }

    chip8->V[x] -= chip8->V[y]; // Odjęcie wartości rejestru Vy od Vx
    chip8->pc += 2; // Normalne przesunięcie na następną instrukcję

    break;

    case 0x0006:
    x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx

    // chip8->V[0xF] = chip8->V[x] & 0x1; // Sprawdzenie najmłodszego bitu i ustawienie flagi VF

    // chip8->V[x] >>= 1; 

			//8xy6 - SHR Vx {, Vy}
			//Set Vx = Vx SHR 1.
			if(chip8->V[x]%2 == 1)
			{
			chip8->V[0xF]= 1;
			}else
			{
		chip8->V[0xF] = 0;
			}

			 chip8->V[x] >>= 1;


    chip8->pc += 2; 



    break;

    case 0x0007:
        x = (opcode & 0x0F00) >> 8;
        y = (opcode & 0x00F0) >> 4;
        chip8->V[0xF] = (chip8->V[y] > chip8->V[x]) ? 1 : 0; // Ustawienie flagi VF na podstawie porównania

        chip8->V[x] = chip8->V[y] - chip8->V[x]; // Wykonanie odejmowania i zapisanie wyniku w Vx
   
        chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
   
    
    break;
case 0x000E:
    x = (opcode & 0x0F00) >> 8;

    chip8->V[0xF] = (chip8->V[x] & 0x80) ? 1 : 0; // Ustawienie flagi VF na podstawie bitu MSB
    chip8->V[x] <<= 1; 
    chip8->pc += 2; 

break;

   }
   break;
   case 0x9000:
        x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
         y = (opcode & 0x00F0) >> 4;// Pobranie numeru rejestru Vy
         if (chip8->V[x] != chip8->V[y]) 
         {
         chip8->pc += 4; // Skok do następnej instrukcji
         }
          else 
         chip8->pc += 2; // Normalne przesunięcie na następną instrukcję
   break;

/*
Annn - LD I, addr
Set I = nnn.

The value of register I is set to nnn.
*/
case 0xA000:
nnn = (opcode & 0x0FFF) ; // Pobranie wartosci nnn
chip8->I = nnn;
chip8->pc += 2;
break;


/*
Bnnn - JP V0, addr
Jump to location nnn + V0.

The program counter is set to nnn plus the value of V0.
*/
case 0xB000:
nnn = (opcode & 0x0FFF) ; // Pobranie wartosci nnn
chip8->pc = nnn+chip8->V[0];
break;

/*
Cxkk - RND Vx, byte
Set Vx = random byte AND kk.

The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx. See instruction 8xy2 for more information on AND.
*/

case 0xC000:
x = (opcode & 0x0F00) >> 8; // Pobranie numeru rejestru Vx
kk = opcode & 0x00FF;  

uint8_t RND = rand() % 256;
RND &= kk;
chip8->V[x] = RND;
chip8->pc += 2;
break;
/*
Dxyn - DRW Vx, Vy, nibble
Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

The interpreter reads n bytes from memory, starting at the address stored in I. These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen. See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.


*/
case 0xD000:
x = (opcode & 0x0F00) >> 8; // Pobranie x
y = (opcode & 0x00F0) >> 4;// Pobranie y
n = (opcode & 0x000F);// Pobranie n
  // Pętla przez wysokość sprite'a
//fprintf(stderr, "robie redraw");
//fprintf(stderr, "\n o wartosci x:");
//fprintf(stderr, "%u",(chip8->V[x]% SCREEN_WIDTH ));
//fprintf(stderr, "\n o wartosci y:");
//fprintf(stderr, "%u",chip8->V[y]);
    for (int row = 0; row < n; row++) {
        uint8_t spriteByte = chip8->memory[chip8->I + row]; // Pobranie bajtu sprite'a z pamięci

        // Pętla przez bity w bajcie sprite'a
        for (int col = 0; col < 8; col++) 
        {
            uint8_t pixelX = (chip8->V[x] + col) % SCREEN_WIDTH; // Owijanie w poziomie
            uint8_t pixelY = (chip8->V[y] + row) % SCREEN_HEIGHT; // Owijanie w pionie

            uint8_t spritePixel = (spriteByte >> (7 - col)) & 0x01; // Pobranie bitu sprite'a

            // XORowanie piksela sprite'a z istniejącym pikselem na ekranie
            if (spritePixel == 1) {
                if (chip8->screen[pixelX + pixelY * SCREEN_WIDTH] == 1) {
                    chip8->V[0xF] = 1; // Ustawienie flagi VF na 1 w przypadku kolizji
                }
                chip8->screen[pixelX + pixelY * SCREEN_WIDTH] ^= 1;
            }
        }
    }

            // unsigned short px;

            // // set collision flag to 0
            // chip8->V[0xF] = 0;

            // // loop over each row
            // for (int yline = 0; yline < n; yline++) 
            // {
            //     // fetch the pixel value from the memory starting at location I
            //     px = chip8->memory[chip8->I + yline];

            //     // loop over 8 bits of one row
            //     for (int xline = 0; xline < 8; xline++) {
            //         // check if current evaluated pixel is set to 1 (0x80 >>
            //         // xline scnas throught the byte, one bit at the time)
            //         if ((px & (0x80 >> xline)) != 0) {
            //             // if drawing causes any pixel to be erased set the
            //             // collision flag to 1
            //             if (chip8->screen[(chip8->V[x] + xline + ((chip8->V[y] + yline) * 64))] ==
            //                 1) {
            //                 chip8->V[0xF] = 1;
            //             }

            //             // set pixel value by using XOR
            //             chip8->screen[chip8->V[x] + xline + ((chip8->V[y] + yline) * 64)] ^= 1;
            //         }
            //     }
            // }



    chip8->drawFlag = 1; // Ustawienie flagi rysowania na ekranie
    chip8->pc += 2; // Przesunięcie na następną instrukcję
    break;

/*
Ex9E - SKP Vx
Skip next instruction if key with the value of Vx is pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
*/
case 0xE000:
switch(opcode&0x00FF)
{
    case 0x009E:
    //fprintf(stderr,"sprawdzam klawisz \n");
    x = (opcode & 0x0F00) >> 8; 
    if (chip8->keys[chip8->V[x]]) 
    { // Check if the key is pressed
        chip8->pc += 4; 
    } else {
        chip8->pc += 2; 
    }
    break;

    case 0x00A1:
    x = (opcode & 0x0F00) >> 8; 
    //fprintf(stderr,"----------sprawdzam czy nie ma jakiegos---------------");
    if (!chip8->keys[chip8->V[x]]) 
    { // Check if the key is pressed
        chip8->pc += 4; 
    } else 
    {
        chip8->pc += 2; 
    }
    break;

    default:
        // Nieznana instrukcja
        chip8->pc += 2;
        printf("Nieznana instrukcja: 0x%X\n", opcode);
break;
}

break;

case 0xF000:
switch(opcode&0x00FF)
{
    case 0x0007:
    x = (opcode & 0x0F00) >> 8; 
    chip8->V[x] = chip8->delay_timer;
    chip8->pc += 2; 
    break;

    case 0x000A:
    x = (opcode & 0x0F00) >> 8; 
    bool wait = true;
    // fprintf(stderr,"czekam na klawisz");
    while(wait)
    for (int i = 0; i < 16; i++) {
        if (chip8->keys[i]) 
        {
            chip8->V[x] = i; // Przypisanie wartości klawisza do Vx
            chip8->pc += 2; // Przesunięcie na następną instrukcję
            wait = false;
            break;
        }
    }
    break;
    case 0x0015:
    // fprintf(stderr,"delay");
    x = (opcode & 0x0F00) >> 8; 
    chip8->delay_timer = chip8->V[x];
    chip8->pc += 2; 
    break;
    case 0x0018:
    x = (opcode & 0x0F00) >> 8; 
    chip8->sound_timer = chip8->V[x];
    chip8->pc += 2; 
    break;

    case 0x001E:
    x = (opcode & 0x0F00) >> 8; 
    chip8->I += chip8->V[x];
    chip8->pc += 2; 
    break;

    case 0x0029:
    x = (opcode & 0x0F00) >> 8; 
    chip8->I = (chip8->V[x]) * 5;
    chip8->pc += 2; 
    break;

    case 0x0033:
    x = (opcode & 0x0F00) >> 8; 
    int val =x;
	int ones = val % 10;
	val /= 10;
	int tens = val % 10;
	val /= 10;
	int hundreds = val % 10;
    chip8->memory[chip8->I] = hundreds;
    chip8->memory[(chip8->I)+1]  = tens;
    chip8->memory[(chip8->I)+2]  =ones;
    chip8->pc += 2; // Przesunięcie na następną instrukcję

    break;
    
   // poprawnie
    // wstaw do pamieci wartosci z rejestru od o do x zaczynajac od I
    
    case 0x0055:
    x = (opcode & 0x0F00) >> 8; 
    for(int i = 0 ;i<=x;i++)
    {
       chip8->memory[chip8->I + i] = chip8->V[i];
    }
    chip8->pc += 2; // Przesunięcie na następną instrukcję

    break;

    // poprawnie
    // wstaw do rejstru od o do x zaczynajc od I w pamieci
    case 0x0065:
    x = (opcode & 0x0F00) >> 8; 
    for(int i = 0 ;i<=x;i++)
    {
      chip8->V[i] = chip8->memory[chip8->I + i];
    }
    chip8->pc += 2; // Przesunięcie na następną instrukcję

    break;

    default:
        // Nieznana instrukcja
        chip8->pc += 2;
        printf("Nieznana instrukcja w ffff: 0x%X\n", opcode);
    break;
}
break;

default:
        // Nieznana instrukcja
        chip8->pc += 2;
        printf("Nieznana instrukcja: 0x%X\n", opcode);
break;


}

updateOutput(chip8,h);

    if (shouldDecrementDelayTimer()) 
    {
        if (chip8->delay_timer > 0) 
        {
            chip8->delay_timer--;
        }
      //  if(chip8->delay_timer==0)
      ///  fprintf(stderr,"koniec");
    }

}


void executeInstruction00E0(uint16_t opcode)
{
int unmasked = opcode & 0x0F00; // unmasked = 0x0C00 = 3072
int result = unmasked >> 8; // result = 0x000C = 12
}

int shouldDecrementDelayTimer(void) {
    static clock_t lastUpdateTime = 0;
    const int delayBetweenDecrementsMs = 16; // 16 ms (60 Hz)

    // Pobranie aktualnego czasu
    clock_t currentTime = clock();

    // Obliczenie różnicy czasu od ostatniej aktualizacji
    double elapsedMs = ((double)(currentTime - lastUpdateTime) / 60) * 1000;

    // Jeśli minął odpowiedni czas, zwróć 1 (true), w przeciwnym razie 0 (false)
    if (elapsedMs >= delayBetweenDecrementsMs) 
    {
        lastUpdateTime = currentTime;
        return 1;
    }

    return 0;
}


int main(int argc, char *argv[]) 
{
    signal(SIGINT, handle_sigint); // Ustawienie obsługi sygnału SIGINT

     file = fopen("output.txt", "w"); // Otwieranie pliku w trybie zapisu
srand(time(NULL));
    if (file == NULL) {
        perror("Nie można otworzyć pliku");
        return 1;
    }


    if (argc != 2) {
        printf("Użycie: %s <plik_chip8>\n", argv[0]);
        return 1;
    }

    Chip8 chip8;
    initializeChip8(&chip8);

    if (!loadProgram(&chip8, argv[1])) {
        printf("Błąd podczas wczytywania pliku: %s\n", argv[1]);
        return 1;
    }

	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO ci;
	GetConsoleCursorInfo(h, &ci);
    ci.bVisible = 0;
    SetConsoleCursorInfo(h, &ci);
    int deltaTime = 0;
	int time1 = clock();
	int time2 = clock();
	int time = 16;

	const int frameDuration = (1.f / 60) * 1000;
    // Główna pętla emulacji
    while (true) {
        //	time2 = clock();
		//deltaTime += time2 - time1;
		//time1 = clock();
	
		//if(deltaTime > frameDuration)
       // {
        emulateCycle(&chip8,h);
        //time -= deltaTime;

      //  deltaTime = 0;
       // }
        // Obsługa wejścia od użytkownika
        // Aktualizacja wyświetlacza
        // ...
    }

    // Zamykanie pliku
    fclose(file);
    return 0;
}
