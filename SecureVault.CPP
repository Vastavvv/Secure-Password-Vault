#include <iostream.h>
#include <fstream.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SIZE 200

// ---------- STRUCTURE ----------
struct Credential {
    char site[50];
    char user[50];
    char enc[SIZE];
};

// ---------- SHA-1 HASHING ----------
typedef unsigned long uint32;

uint32 rol(uint32 value, int bits) {
    return ((value << bits) | (value >> (32 - bits)));
}

void sha1_hash(char *message, char out_hex[]) {
    unsigned char msg[64];
    int i;

    for (i = 0; i < 64; i++)
        msg[i] = 0;

    int len = strlen(message);
    for (i = 0; i < len; i++)
        msg[i] = (unsigned char)message[i];

    msg[len] = 0x80;
    unsigned long bit_len = (unsigned long)len * 8;

    msg[60] = (bit_len >> 24) & 0xFF;
    msg[61] = (bit_len >> 16) & 0xFF;
    msg[62] = (bit_len >> 8)  & 0xFF;
    msg[63] = (bit_len)       & 0xFF;

    uint32 h0 = 0x67452301;
    uint32 h1 = 0xEFCDAB89;
    uint32 h2 = 0x98BADCFE;
    uint32 h3 = 0x10325476;
    uint32 h4 = 0xC3D2E1F0;

    uint32 w[80];

    for (i = 0; i < 16; i++)
        w[i] = (msg[i*4] << 24) | (msg[i*4+1] << 16) | (msg[i*4+2] << 8) | (msg[i*4+3]);

    for (i = 16; i < 80; i++)
        w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

    uint32 a = h0, b = h1, c = h2, d = h3, e = h4;

    for (i = 0; i < 80; i++) {
        uint32 f, k;

        if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
        else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
        else { f = b ^ c ^ d; k = 0xCA62C1D6; }

        uint32 temp = rol(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = rol(b, 30);
        b = a;
        a = temp;
    }

    h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;

    sprintf(out_hex, "%08lx%08lx%08lx%08lx%08lx", h0, h1, h2, h3, h4);
}

// ---------- FILE HANDLING MASTER PASSWORD ----------
void save_master_hash(char h[]) {
    ofstream f("master.txt");
    f << h;
    f.close();
}

void load_master_hash(char h[]) {
    ifstream f("master.txt");
    if (!f) { h[0] = '\0'; return; }
    f >> h;
    f.close();
}

// ---------- RSA ----------
int p, q, n, phi, e, d;

long long power(long long base, long long exp) {
    long long res = 1;
    for (int i = 0; i < exp; i++)
        res = (res * base) % n;
    return res;
}

void generate_rsa_keys() {
    p = 17; q = 11;
    n = p * q;
    phi = (p - 1) * (q - 1);
    e = 7; d = 23;
}

void rsa_encrypt(char msg[], char enc[]) {
    char buffer[20];
    enc[0] = '\0';
    for (int i = 0; i < strlen(msg); i++) {
        long long encrypted_char = power(msg[i], e);
        sprintf(buffer, "%ld ", encrypted_char);
        strcat(enc, buffer);
    }
}

void rsa_decrypt(char enc[], char dec[]) {
    dec[0] = '\0';
    char *token = strtok(enc, " ");
    while (token != NULL) {
        long v = atol(token);
        char ch = (char)power(v, d);
        int len = strlen(dec);
        dec[len] = ch;
        dec[len+1] = '\0';
        token = strtok(NULL, " ");
    }
}

// ---------- BORDER ----------
void draw_border(int color) {
    textcolor(color);
    for (int i = 1; i < 80; i++) {
        gotoxy(i,1); cprintf("%c",219);
        gotoxy(i,25); cprintf("%c",219);
    }
    for (int k = 1; k < 25; k++) {
        gotoxy(1,k); cprintf("%c",219);
        gotoxy(80,k); cprintf("%c",219);
    }
}

// ---------- ADD ----------
void add_credential() {
    clrscr();
    draw_border(LIGHTBLUE);
    textcolor(LIGHTBLUE);
    gotoxy(25, 3);
    cprintf("=== ADD NEW CREDENTIAL ===");

    Credential c;
    char pwd[50];

    textcolor(WHITE);
    gotoxy(10,6); cout << "Enter Site: "; fflush(stdin); gets(c.site);
    gotoxy(10,8); cout << "Enter Username: "; fflush(stdin); gets(c.user);
    gotoxy(10,10); cout << "Enter Password: "; fflush(stdin); gets(pwd);

    rsa_encrypt(pwd, c.enc);
    ofstream f("vault.dat", ios::binary | ios::app);
    f.write((char*)&c, sizeof(c));
    f.close();

    gotoxy(10,14);
    textcolor(LIGHTGREEN);
    cout << "Credential Saved!";
    getch();
}

// ---------- VIEW ----------
void view_vault() {
    clrscr();
    draw_border(LIGHTGREEN);
    textcolor(LIGHTGREEN);
    gotoxy(25,3);
    cprintf("=== VIEW ALL CREDENTIALS ===");

    ifstream f("vault.dat", ios::binary);
    if (!f) { gotoxy(10,8); cout << "Vault Empty."; getch(); return; }

    Credential c;
    char enc_copy[SIZE], dec[SIZE];
    int count = 1, y = 6;

    textcolor(YELLOW);
    gotoxy(5,5);
    cprintf("No  Site            Username       Password");

    while (f.read((char*)&c, sizeof(c))) {
        strcpy(enc_copy, c.enc);
        rsa_decrypt(enc_copy, dec);
        textcolor(WHITE);
        gotoxy(5,y);
        cprintf("%2d. %-15s %-14s %-15s", count++, c.site, c.user, dec);
        y++;
    }
    f.close();
    getch();
}

// ---------- SEARCH ----------
void search_credential() {
    clrscr();
    draw_border(CYAN);
    textcolor(CYAN);
    gotoxy(25,3);
    cprintf("=== SEARCH CREDENTIAL ===");

    char target[50];
    gotoxy(10,6);
    textcolor(WHITE);
    cout << "Enter site name: ";
    fflush(stdin);
    gets(target);

    ifstream f("vault.dat", ios::binary);
    if (!f) { gotoxy(10,10); cout << "Vault Empty."; getch(); return; }

    Credential c;
    char enc_copy[SIZE], dec[SIZE];
    int found = 0, y = 9;

    while (f.read((char*)&c, sizeof(c))) {
        if (strcmpi(c.site, target) == 0) {
            strcpy(enc_copy, c.enc);
            rsa_decrypt(enc_copy, dec);

            gotoxy(10, y++); cout << "Site     : " << c.site;
            gotoxy(10, y++); cout << "Username : " << c.user;
            gotoxy(10, y++); cout << "Password : " << dec;
            y++;
            found = 1;
        }
    }
    if (!found) { gotoxy(10,9); cout << "No record found."; }
    getch();
}

// ---------- UPDATE ----------
void update_credential() {
    clrscr();
    draw_border(LIGHTMAGENTA);
    textcolor(LIGHTMAGENTA);
    gotoxy(25,3); cprintf("=== UPDATE CREDENTIAL ===");

    char site[50], user[50];
    textcolor(WHITE);
    gotoxy(10,6); cout << "Enter site: "; fflush(stdin); gets(site);
    gotoxy(10,8); cout << "Enter username: "; fflush(stdin); gets(user);

    ifstream f("vault.dat", ios::binary);
    ofstream t("temp.dat", ios::binary);
    if (!f) { gotoxy(10,10); cout << "Vault Empty."; getch(); return; }

    Credential c;
    char new_user[50], new_pwd[50];
    int found = 0;

    while (f.read((char*)&c, sizeof(c))) {
        if (strcmpi(c.site, site) == 0 && strcmpi(c.user, user) == 0) {
            found = 1;
            gotoxy(10,11); cout << "Enter new username: "; fflush(stdin); gets(new_user);
            gotoxy(10,12); cout << "Enter new password: "; fflush(stdin); gets(new_pwd);
            strcpy(c.user, new_user);
            rsa_encrypt(new_pwd, c.enc);
        }
        t.write((char*)&c, sizeof(c));
    }

    f.close(); t.close();
    remove("vault.dat");
    rename("temp.dat", "vault.dat");

    gotoxy(10,15);
    if (found) cout << "Updated Successfully!";
    else cout << "Record Not Found.";
    getch();
}

// ---------- DELETE ----------
void delete_credential() {
    clrscr();
    draw_border(LIGHTRED);
    textcolor(LIGHTRED);
    gotoxy(25,3); cprintf("=== DELETE CREDENTIAL ===");

    char site[50], user[50];
    gotoxy(10,6); textcolor(WHITE);
    cout << "Enter site: "; fflush(stdin); gets(site);
    gotoxy(10,8); cout << "Enter username: "; fflush(stdin); gets(user);

    ifstream f("vault.dat", ios::binary);
    ofstream t("temp.dat", ios::binary);
    if (!f) { gotoxy(10,10); cout << "Vault Empty."; getch(); return; }

    Credential c;
    int found = 0;

    while (f.read((char*)&c, sizeof(c))) {
        if (!(strcmpi(c.site, site) == 0 && strcmpi(c.user, user) == 0))
            t.write((char*)&c, sizeof(c));
        else
            found = 1;
    }

    f.close(); t.close();
    remove("vault.dat");
    rename("temp.dat", "vault.dat");

    gotoxy(10,13);
    if (found) cout << "Deleted!";
    else cout << "Record Not Found!";
    getch();
}

// ---------- UPDATE MASTER PASSWORD ----------
void update_master_password() {
    clrscr();
    draw_border(LIGHTMAGENTA);
    textcolor(LIGHTMAGENTA);
    gotoxy(20,3);
    cprintf("=== UPDATE MASTER PASSWORD ===");

    char current_pwd[50], new_pwd[50];
    char stored_hash[100], temp_hash[100];

    load_master_hash(stored_hash);

    gotoxy(10,6); cout << "Enter current master password: "; fflush(stdin); gets(current_pwd);
    sha1_hash(current_pwd, temp_hash);

    if (strcmp(temp_hash, stored_hash) != 0) {
        gotoxy(10,8); cout << "Incorrect password!";
        getch();
        return;
    }

    gotoxy(10,9); cout << "Enter new master password: "; fflush(stdin); gets(new_pwd);
    sha1_hash(new_pwd, stored_hash);
    save_master_hash(stored_hash);

    gotoxy(10,11); cout << "Master password updated!";
    getch();
}

// ---------- MAIN ----------
void main() {
    clrscr();
    textbackground(BLACK);
    textcolor(LIGHTCYAN);
    draw_border(LIGHTCYAN);

    gotoxy(25,3);
    cprintf("SECURE PASSWORD VAULT SYSTEM");
    gotoxy(22,5);
    cprintf("Developed with RSA + SHA-1 Security");

    generate_rsa_keys();

    char master_pwd[50], stored_hash[100];
    ifstream check("master.txt");

    if (!check) {
        gotoxy(10,10);
        cout << "First time setup. Create master password: ";
        fflush(stdin); gets(master_pwd);
        sha1_hash(master_pwd, stored_hash);
        save_master_hash(stored_hash);
        cout << "\nSetup complete!";
        getch();
    } else {
        load_master_hash(stored_hash);
    }

    int tries = 0;
    char temp_hash[100];
    int ok = 0;

    while (tries < 3 && !ok) {
        clrscr();
        draw_border(LIGHTCYAN);
        gotoxy(25,4); cprintf("--- SECURE VAULT LOGIN ---");
        gotoxy(20,10); cout << "Enter master password: ";
        fflush(stdin); gets(master_pwd);

        sha1_hash(master_pwd, temp_hash);
        if (strcmp(temp_hash, stored_hash) == 0)
            ok = 1;
        else {
            gotoxy(20,12); cout << "Wrong password!";
            tries++; getch();
        }
    }

    if (!ok) { clrscr(); cout << "Too many attempts."; return; }

    int choice;
    while (1) {
        clrscr();
        draw_border(LIGHTCYAN);
        gotoxy(25,3); cprintf("=== SECURE VAULT MENU ===");
        gotoxy(25,6); cout << "1. Add Credential";
        gotoxy(25,7); cout << "2. View All";
        gotoxy(25,8); cout << "3. Search";
        gotoxy(25,9); cout << "4. Update";
        gotoxy(25,10); cout << "5. Delete";
        gotoxy(25,11); cout << "6. Update Master Password";
        gotoxy(25,12); cout << "7. Exit";
        gotoxy(25,14); cout << "Enter choice: ";
        cin >> choice; cin.ignore();

        switch (choice) {
            case 1: add_credential(); break;
            case 2: view_vault(); break;
            case 3: search_credential(); break;
            case 4: update_credential(); break;
            case 5: delete_credential(); break;
            case 6: update_master_password(); break;
            case 7: clrscr(); cout << "Thank you for using Secure Vault!"; getch(); return;
            default: gotoxy(25,16); cout << "Invalid choice!"; getch();
        }
    }
}
