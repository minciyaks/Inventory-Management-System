
# Inventory Management System in C

A menu-driven **Inventory Management System** developed using the C programming language as an academic project. The application was originally designed and executed using **Turbo C 3.0**.

The system provides facilities to manage items, record purchases, generate sales bills, monitor stock levels and view inventory reports through a text-based user interface.

## Project Features

### 1. Item Management
- Add new items.
- Edit existing item details.
- Delete items.
- View item inventory.
- Search items using item codes.
- Maintain opening stock and current stock.

### 2. Purchase Management
- Record purchases with automatically generated purchase numbers.
- Add multiple products to a purchase.
- Edit purchase product details.
- Delete individual purchase lines or entire purchases.
- View purchase history.
- Automatically increase stock after purchases.

### 3. Sales Management
- Generate sales bills containing multiple products.
- Automatically generate bill numbers.
- Edit sales bill product details.
- Delete individual sales lines or entire bills.
- View sales history and bill details.
- Check available stock before completing a sale.
- Automatically decrease stock after sales.

### 4. Tax Calculation
- Calculates sales tax at a rate of 5%.
- Calculates tax on the complete bill subtotal.
- Distributes the calculated tax across individual product lines.
- Displays subtotal, tax and final bill amount.

### 5. Reports
- **Low Stock Report:** Displays items at or below a user-defined stock threshold.
- **Inventory Summary:** Displays total item count, stock units, low-stock items, out-of-stock items and inventory value.

## Technologies Used

- **Programming Language:** C
- **Compiler:** Turbo C 3.0
- **Header Files:** `stdio.h`, `conio.h`, `string.h`, `stdlib.h`
- **Storage:** Binary File Handling
- **Interface:** Text-based Console Interface

## C Programming Concepts Used

- Structures
- Arrays
- User-defined functions
- Conditional statements
- Switch statements
- Loops (`for`, `while`, `do-while`)
- String handling
- File handling (`fopen`, `fread`, `fwrite`, `fclose`)
- Data validation

## Data Storage

The application stores information in binary files in the program's current working directory.

| File | Purpose |
|---|---|
| `ITEMS.DAT` | Stores item information and stock details |
| `PURCHASE.DAT` | Stores purchase records |
| `SALES.DAT` | Stores sales and bill records |
| `PURCHASE.NXT` | Stores the next purchase number |
| `SALES.NXT` | Stores the next bill number |

The application supports up to 300 item records, 300 purchase records and 300 sales records.

## How to Run

This project was originally developed for **Turbo C 3.0**.

1. Install or open Turbo C 3.0.
2. Open the project C source file.
3. Compile the program using `Alt + F9`.
4. Run the program using `Ctrl + F9`.
5. Use the displayed menus to manage items, purchases and sales.

### Compatibility

The original source uses Turbo C-specific functions from `conio.h`, including:

- `clrscr()`
- `gotoxy()`
- `textcolor()`
- `textbackground()`
- `cprintf()`
- `getch()`

It is intended for Turbo C 3.0 and is not directly portable to modern GCC compilers without modifications.

## Project Screenshots

Add screenshots of the actual program output here.

Example:

```text
Screenshots/
├── Main-Menu.png
├── Item-Management.png
├── Purchase-Management.png
├── Sales-Management.png
└── Sales-History.png
```

## Project Structure

```text
Inventory-Management-System/
│
├── Inventory_Management_System.C
├── Screenshots/
│   ├── Main-Menu.png
│   ├── Item-Management.png
│   ├── Purchase-Management.png
│   ├── Sales-Management.png
│   └── Sales-History.png
│
└── README.md
```

## Project Purpose

This project was developed to apply fundamental C programming concepts to a practical inventory management scenario. It demonstrates how structures, functions, arrays and file handling can be combined to create a menu-driven application.

## License

This project is available for educational and learning purposes.
