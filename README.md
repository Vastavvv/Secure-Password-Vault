# Secure Password Vault System

A Turbo C++ based offline password management system that securely stores user credentials using RSA encryption, SHA-1 hashing, and binary file handling.

## Features

- Master password authentication
- SHA-1 hashing for master password
- RSA encryption and decryption
- Binary file storage using `vault.dat`
- Add credentials
- View all credentials
- Search credentials
- Update credentials
- Delete credentials
- Update master password
- Three login attempts
- Console-based user interface

## Technologies Used

- C++
- Turbo C++
- RSA Encryption
- SHA-1 Hashing
- Binary File Handling
- File I/O
- Structures

## File Storage

The project uses:

- `vault.dat` — stores credential records in binary format
- `master.txt` — stores the SHA-1 hash of the master password

Runtime data files containing actual credentials are not included in the repository.

## How It Works

1. User creates a master password.
2. The master password is converted into a SHA-1 hash.
3. The user logs into the vault.
4. Passwords entered for websites are encrypted using RSA.
5. Encrypted credentials are stored in `vault.dat`.
6. Search/View operations decrypt the stored password when required.
7. Update and Delete operations identify records using site and username.

## RSA Key Parameters

The educational implementation uses:

- p = 17
- q = 11
- n = 187
- e = 7
- d = 23

## Project Type

Educational / Academic Cryptography Project

## Author

Vastav
