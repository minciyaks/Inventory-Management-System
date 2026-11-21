/* Current one INVENTORY MANAGEMENT SYSTEM - Turbo C 3.0
   Uses conio.h screen controls and binary files in the current directory.
   Tax rate: 5%; b_tax stores the calculated tax amount.
*/
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

#define MAXREC 300
#define BOX_LEFT 10
#define TAX_RATE 5
#define ITEMFILE "ITEMS.DAT"
#define PURFILE "PURCHASE.DAT"
#define SALESFILE "SALES.DAT"
#define PURNOFILE "PURCHASE.NXT"
#define BILLNOFILE "SALES.NXT"

struct Item {
    int it_code;
    char it_name[20];
    int it_price;
    int it_os;
    int it_cs;
};
struct Purchase {
    int p_no;
    char p_date[20];
    int c_code;
    char c_name[20];
    int p_code;
    int p_qty;
    int p_price;
    int p_amt;
};
struct Sales {
    int b_no;
    char b_date[11];
    int p_code;
    int b_qty;
    int b_rate;
    int b_tax;
    int b_amt;
};

struct Item items[MAXREC];
struct Purchase purchases[MAXREC];
struct Sales sales[MAXREC];
int itemCount=0, purchaseCount=0, salesCount=0;
int nextPurchaseNumber=1, nextBillNumber=1;

/* ---------- Three-section box interface ---------- */
void drawHorizontal(int row, int left, int middle, int right)
{
    int col;
    gotoxy(BOX_LEFT,row);
    textcolor(LIGHTCYAN);
    cprintf("%c",left);
    for(col=0; col<60; col++)
        cprintf("%c",middle);
    cprintf("%c",right);
}

void centerHeader(int row, char *s)
{
    int x;
    x = BOX_LEFT + 1 + (60 - strlen(s)) / 2;
    gotoxy(x,row);
    textcolor(LIGHTCYAN);
    cprintf("%s",s);
}

void frame(void)
{
    int r;
    textbackground(BLACK);
    clrscr();
    textcolor(LIGHTCYAN);

    /* 62-column double-line box centered on an 80-column DOS screen. */
    drawHorizontal(1,201,205,187);
    gotoxy(BOX_LEFT,2); cprintf("%c",186);
    centerHeader(2,"INVENTORY MANAGEMENT SYSTEM");
    gotoxy(BOX_LEFT+61,2); cprintf("%c",186);

    gotoxy(BOX_LEFT,3); cprintf("%c",186);
    centerHeader(3,"STOCK | PURCHASE | SALES");
    gotoxy(BOX_LEFT+61,3); cprintf("%c",186);

    drawHorizontal(4,204,205,185);

    for(r=5;r<=15;r++) {
        gotoxy(BOX_LEFT,r);
        cprintf("%c                                                            %c",186,186);
    }

    drawHorizontal(16,204,205,185);

    gotoxy(BOX_LEFT,17);
    cprintf("%c INPUT                                                      %c",186,186);
    gotoxy(BOX_LEFT,18);
    cprintf("%c                                                            %c",186,186);
    gotoxy(BOX_LEFT,19);
    cprintf("%c                                                            %c",186,186);

    drawHorizontal(20,200,205,188);
}
void clearContent(void)
{
    int r;
    textcolor(WHITE);
    for(r=5;r<=15;r++) {
        gotoxy(BOX_LEFT+1,r);
        cprintf("                                                            ");
    }
}
void lineOut(int r, char *s)
{
    gotoxy(BOX_LEFT+2,r);
    textcolor(WHITE);
    cprintf("%-58.58s",s);
}
void lineOutColor(int r, char *s, int color)
{
    gotoxy(BOX_LEFT+2,r);
    textcolor(color);
    cprintf("%-58.58s",s);
}
void prompt(char *s)
{
    int x;
    /* Clear the prompt row, then place the cursor immediately after the question. */
    gotoxy(BOX_LEFT+1,18);
    textcolor(BLACK);
    cprintf("                                                            ");
    gotoxy(BOX_LEFT+1,18);
    textcolor(YELLOW);
    cprintf("%s",s);
    x = BOX_LEFT + 1 + strlen(s);
    gotoxy(x,18);
    textcolor(WHITE);
}
void readText(char *p, char *s, int size)
{
    int i,ch;
    prompt(p); i=0; s[0]='\0';
    while(i<size-1) {
        ch=getch();
        if(ch==13) break;
        if(ch==8) {
            if(i>0) { i--; s[i]='\0'; cprintf("\b \b"); }
        } else if(ch>=32 && ch<=126) {
            s[i]=(char)ch; i++; s[i]='\0'; putch(ch);
        }
    }
    s[i]='\0';
}
int readInt(char *p)
{
    char b[20];
    readText(p,b,19);
    return atoi(b);
}
int confirm(char *p)
{
    char b[5];
    readText(p,b,4);
    return (b[0]=='Y' || b[0]=='y');
}

/* Validate a real calendar date in DD-MM-YYYY format. */
int validDate(char *date)
{
    int d,m,y,days,leap;
    if(strlen(date)!=10 || date[2]!='-' || date[5]!='-') return 0;
    if(date[0]<'0'||date[0]>'9'||date[1]<'0'||date[1]>'9'||
       date[3]<'0'||date[3]>'9'||date[4]<'0'||date[4]>'9') return 0;
    if(date[6]<'0'||date[6]>'9'||date[7]<'0'||date[7]>'9'||
       date[8]<'0'||date[8]>'9'||date[9]<'0'||date[9]>'9') return 0;
    d=(date[0]-'0')*10+(date[1]-'0');
    m=(date[3]-'0')*10+(date[4]-'0');
    y=(date[6]-'0')*1000+(date[7]-'0')*100+(date[8]-'0')*10+(date[9]-'0');
    if(y<1900 || m<1 || m>12 || d<1) return 0;
    leap=(y%400==0 || (y%4==0 && y%100!=0));
    if(m==4||m==6||m==9||m==11) days=30;
    else if(m==2) days=leap?29:28;
    else days=31;
    return d<=days;
}


/* Case 2: calculate 5% tax once on the entire bill subtotal. Store that bill tax
   across its lines proportionally so existing sales-history fields remain useful.
   The final line receives any rounding remainder, ensuring line taxes sum exactly
   to the rounded bill-level tax. */
void recalculateBillTax(int billNo)
{
    int i,last=-1;
    long subtotal=0,totalTax,allocated=0,lineBase;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==billNo) {
        lineBase=(long)sales[i].b_qty*sales[i].b_rate;
        subtotal+=lineBase;
        last=i;
    }
    if(last<0) return;
    totalTax=(subtotal*TAX_RATE+50L)/100L;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==billNo) {
        lineBase=(long)sales[i].b_qty*sales[i].b_rate;
        if(i==last) sales[i].b_tax=(int)(totalTax-allocated);
        else {
            sales[i].b_tax=(subtotal>0)?(int)((lineBase*totalTax)/subtotal):0;
            allocated+=sales[i].b_tax;
        }
        sales[i].b_amt=(int)(lineBase+sales[i].b_tax);
    }
}

/* Round a 5% tax amount to the nearest whole rupee. */
int calculateTax(int quantity, int rate)
{
    long base;
    base=(long)quantity*(long)rate;
    return (int)((base*TAX_RATE+50L)/100L);
}

void pauseScreen(void)
{
    prompt("Press any key to continue...");
    getch();
}
void message(char *s, int ok)
{
    frame(); clearContent();
    if(ok) textcolor(LIGHTGREEN); else textcolor(LIGHTRED);
    lineOut(9,s);
    pauseScreen();
}

/* ---------- Load/save files ---------- */
void loadItems(void)
{
    FILE *f;
    itemCount=0; f=fopen(ITEMFILE,"rb");
    if(f) {
        while(itemCount<MAXREC && fread(&items[itemCount],sizeof(struct Item),1,f)==1)
            itemCount++;
        fclose(f);
    }
}
void saveItems(void)
{
    FILE *f=fopen(ITEMFILE,"wb");
    if(!f) { message("Could not save ITEMS.DAT.",0); return; }
    if(itemCount) fwrite(items,sizeof(struct Item),itemCount,f);
    fclose(f);
}
void loadPurchases(void)
{
    FILE *f;
    purchaseCount=0; f=fopen(PURFILE,"rb");
    if(f) {
        while(purchaseCount<MAXREC && fread(&purchases[purchaseCount],sizeof(struct Purchase),1,f)==1)
            purchaseCount++;
        fclose(f);
    }
}
void savePurchases(void)
{
    FILE *f=fopen(PURFILE,"wb");
    if(!f) { message("Could not save PURCHASE.DAT.",0); return; }
    if(purchaseCount) fwrite(purchases,sizeof(struct Purchase),purchaseCount,f);
    fclose(f);
}
void loadSales(void)
{
    FILE *f;
    salesCount=0; f=fopen(SALESFILE,"rb");
    if(f) {
        while(salesCount<MAXREC && fread(&sales[salesCount],sizeof(struct Sales),1,f)==1)
            salesCount++;
        fclose(f);
    }
}
void saveSales(void)
{
    FILE *f=fopen(SALESFILE,"wb");
    if(!f) { message("Could not save SALES.DAT.",0); return; }
    if(salesCount) fwrite(sales,sizeof(struct Sales),salesCount,f);
    fclose(f);
}
/* Read/write the next transaction numbers. These small files ensure that
   deleted purchase/bill numbers are not reused after restarting the program. */
int highestPurchaseNumber(void)
{
    int i, highest=0;
    for(i=0;i<purchaseCount;i++)
        if(purchases[i].p_no>highest) highest=purchases[i].p_no;
    return highest;
}
int highestBillNumber(void)
{
    int i, highest=0;
    for(i=0;i<salesCount;i++)
        if(sales[i].b_no>highest) highest=sales[i].b_no;
    return highest;
}
int loadNextNumber(char *filename, int highest)
{
    FILE *f;
    int next=highest+1;
    f=fopen(filename,"rb");
    if(f) {
        if(fread(&next,sizeof(int),1,f)!=1) next=highest+1;
        fclose(f);
    }
    if(next<=highest || next<1) next=highest+1;
    return next;
}
void saveNextNumber(char *filename, int next)
{
    FILE *f=fopen(filename,"wb");
    if(f) { fwrite(&next,sizeof(int),1,f); fclose(f); }
}
void loadAll(void)
{
    loadItems();
    loadPurchases();
    loadSales();
    nextPurchaseNumber=loadNextNumber(PURNOFILE,highestPurchaseNumber());
    nextBillNumber=loadNextNumber(BILLNOFILE,highestBillNumber());
}

/* ---------- Find records ---------- */
int itemIndex(int code)
{
    int i;
    for(i=0;i<itemCount;i++) if(items[i].it_code==code) return i;
    return -1;
}
int purchaseIndex(int no)
{
    int i;
    for(i=0;i<purchaseCount;i++) if(purchases[i].p_no==no) return i;
    return -1;
}
int saleIndex(int no)
{
    int i;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==no) return i;
    return -1;
}

/* ---------- Item functions ---------- */
void addItem(void)
{
    struct Item t;
    frame(); clearContent(); lineOutColor(6,"ADD NEW ITEM",BROWN);
    if(itemCount>=MAXREC) { message("Item file is full.",0); return; }
    t.it_code=readInt("Item code: ");
    if(t.it_code<=0 || itemIndex(t.it_code)>=0) { message("Invalid or duplicate item code.",0); return; }
    readText("Item name: ",t.it_name,20);
    if(t.it_name[0]=='\0') { message("Item name cannot be empty.",0); return; }
    t.it_price=readInt("Price (whole rupees): ");
    t.it_os=readInt("Opening stock: ");
    if(t.it_price<0 || t.it_os<0) { message("Price and stock cannot be negative.",0); return; }
    t.it_cs=t.it_os;
    items[itemCount++]=t; saveItems();
    message("Item added successfully.",1);
}
void viewItems(void)
{
    int i,start,end,row;
    if(itemCount==0) { message("No item records found.",0); return; }
    start=0;
    while(start<itemCount) {
        frame(); clearContent();
        lineOutColor(5,"ITEM INVENTORY",BROWN);
        lineOut(6,"CODE  ITEM NAME           PRICE   OPENING   CURRENT");
        lineOut(7,"----------------------------------------------------");
        end=start+6; if(end>itemCount) end=itemCount; row=9;
        for(i=start;i<end;i++,row++) {
            gotoxy(BOX_LEFT+2,row); textcolor(WHITE);
            cprintf("%-5d %-20.20s %-7d %-9d %-7d",
                items[i].it_code,items[i].it_name,items[i].it_price,items[i].it_os,items[i].it_cs);
        }
        pauseScreen(); start=end;
    }
}
void searchItem(void)
{
    int code,k; char b[80];
    frame(); clearContent(); lineOutColor(6,"SEARCH ITEM",BROWN);
    code=readInt("Enter item code: "); k=itemIndex(code);
    if(k<0) { message("Item not found.",0); return; }
    frame(); clearContent(); lineOutColor(6,"ITEM FOUND",BROWN);
    sprintf(b,"Item code     : %d",items[k].it_code); lineOut(8,b);
    sprintf(b,"Item name     : %s",items[k].it_name); lineOut(9,b);
    sprintf(b,"Item price    : %d",items[k].it_price); lineOut(10,b);
    sprintf(b,"Opening stock : %d",items[k].it_os); lineOut(11,b);
    sprintf(b,"Current stock : %d",items[k].it_cs); lineOut(12,b);
    pauseScreen();
}
void editItem(void)
{
    int code,k,price; char name[20],b[80];
    frame(); clearContent(); lineOutColor(5,"EDIT ITEM",BROWN);
    code=readInt("Item code to edit: "); k=itemIndex(code);
    if(k<0) { message("Item not found.",0); return; }

    /* Show the current details before asking for changes. */
    frame(); clearContent(); lineOutColor(5,"CURRENT ITEM DETAILS",BROWN);
    sprintf(b,"Item code     : %d",items[k].it_code); lineOut(7,b);
    sprintf(b,"Item name     : %s",items[k].it_name); lineOut(8,b);
    sprintf(b,"Current price : %d",items[k].it_price); lineOut(9,b);
    sprintf(b,"Current stock : %d",items[k].it_cs); lineOut(10,b);
    lineOut(12,"Leave name blank to keep it unchanged.");
    lineOut(13,"Enter price 0 to keep current price.");

    readText("New item name: ",name,20);
    price=readInt("New item price: ");
    if(price<0) { message("Price cannot be negative.",0); return; }
    if(name[0]!='\0') strcpy(items[k].it_name,name);
    if(price>0) items[k].it_price=price;
    saveItems(); message("Item updated.",1);
}
void deleteItem(void)
{
    int code,k,i,hasPurchase,hasSale;
    char b[80];
    frame(); clearContent(); lineOutColor(5,"DELETE ITEM",BROWN);
    code=readInt("Item code to delete: "); k=itemIndex(code);
    if(k<0) { message("Item not found.",0); return; }

    frame(); clearContent(); lineOutColor(5,"ITEM SELECTED FOR DELETION",BROWN);
    sprintf(b,"Item code     : %d",items[k].it_code); lineOut(7,b);
    sprintf(b,"Item name     : %s",items[k].it_name); lineOut(8,b);
    sprintf(b,"Selling price : %d",items[k].it_price); lineOut(9,b);
    sprintf(b,"Opening stock : %d",items[k].it_os); lineOut(10,b);
    sprintf(b,"Current stock : %d",items[k].it_cs); lineOut(11,b);

    hasPurchase=0; hasSale=0;
    for(i=0;i<purchaseCount;i++)
        if(purchases[i].p_code==code) hasPurchase=1;
    for(i=0;i<salesCount;i++)
        if(sales[i].p_code==code) hasSale=1;

    if(hasPurchase || hasSale) {
        lineOutColor(13,"Deletion blocked: transaction history exists.",LIGHTRED);
        if(hasPurchase) lineOut(14,"Reason: this item appears in purchase history.");
        if(hasSale) lineOut(15,"Reason: this item appears in sales history.");
        pauseScreen();
        return;
    }
    if(!confirm("Delete this item? Type Y: ")) { message("Deletion cancelled.",1); return; }
    for(i=k;i<itemCount-1;i++) items[i]=items[i+1];
    itemCount--; saveItems(); message("Item deleted.",1);
}
void itemMenu(void)
{
    int c;
    do {
        frame(); clearContent(); lineOutColor(6,"ITEM MANAGEMENT",BROWN);
        lineOut(8,"1. Add Item"); lineOut(9,"2. Edit Item");
        lineOut(10,"3. Delete Item"); lineOut(11,"4. View Items");
        lineOut(12,"5. Search Item"); lineOut(13,"6. Back");
        c=readInt("Item menu choice: ");
        switch(c) {
            case 1:addItem();break; case 2:editItem();break;
            case 3:deleteItem();break; case 4:viewItems();break;
            case 5:searchItem();break; case 6:break;
            default:message("Invalid choice.",0);
        }
    } while(c!=6);
}

/* ---------- Purchase functions ---------- */
void addPurchase(void)
{
    struct Purchase t;
    int k, more, lines, total, i, duplicate;
    char b[80];

    if(purchaseCount>=MAXREC) {
        message("Purchase file is full.",0);
        return;
    }

    frame();
    clearContent();
    lineOutColor(5,"CREATE PURCHASE",BROWN);
    t.p_no=nextPurchaseNumber++;
    saveNextNumber(PURNOFILE,nextPurchaseNumber);
    {
        char numberText[80];
        sprintf(numberText,"Purchase number assigned automatically: %d",t.p_no);
        lineOutColor(6,numberText,WHITE);
    }

    do {
        readText("Date (DD-MM-YYYY): ",t.p_date,20);
        if(!validDate(t.p_date)) message("Invalid date. Use a real DD-MM-YYYY date.",0);
    } while(!validDate(t.p_date));
    t.c_code=readInt("Client code: ");
    readText("Client name: ",t.c_name,20);

    lines=0;
    total=0;
    more=1;

    while(more==1 && purchaseCount<MAXREC) {
        frame();
        clearContent();

        lineOutColor(5,"CREATE PURCHASE",BROWN);
        sprintf(b,"Purchase No: %d   Client: %.20s",t.p_no,t.c_name);
        lineOutColor(6,b,WHITE);
        sprintf(b,"PRODUCT %d",lines+1);
        lineOutColor(7,b,BROWN);

        t.p_code=readInt("Product code: ");
        k=itemIndex(t.p_code);

        if(k<0) {
            message("Item code not found. Product was not added.",0);
            if(lines==0) return;
            more=confirm("Add another product? (Y/N): ");
            continue;
        }

        duplicate=0;
        for(i=0;i<purchaseCount;i++) {
            if(purchases[i].p_no==t.p_no &&
               purchases[i].p_code==t.p_code) {
                duplicate=1;
                break;
            }
        }
        if(duplicate) {
            message("This item is already in this purchase.",0);
            more=confirm("Add a different product? (Y/N): ");
            continue;
        }

        sprintf(b,"ITEM: %.20s",items[k].it_name);
        lineOutColor(8,b,WHITE);
        sprintf(b,"AVAILABLE STOCK: %d",items[k].it_cs);
        lineOutColor(9,b,WHITE);
        sprintf(b,"STANDARD PRICE: %d",items[k].it_price);
        lineOutColor(10,b,WHITE);

        t.p_qty=readInt("Quantity: ");
        t.p_price=readInt("Purchase price: ");
        if(t.p_qty<=0 || t.p_price<0) {
            message("Quantity must be positive; price cannot be negative.",0);
            more=confirm("Try another product? (Y/N): ");
            continue;
        }

        t.p_amt=t.p_qty*t.p_price;
        if(purchaseCount>=MAXREC) {
            message("Purchase file is full.",0);
            break;
        }

        items[k].it_cs+=t.p_qty;
        purchases[purchaseCount]=t;
        purchaseCount++;
        saveItems();
        savePurchases();

        total+=t.p_amt;
        lines++;

        frame();
        clearContent();
        lineOutColor(5,"PRODUCT ADDED",BROWN);
        sprintf(b,"ITEM: %.20s",items[k].it_name);
        lineOutColor(7,b,WHITE);
        sprintf(b,"QUANTITY: %d   LINE AMOUNT: %d",t.p_qty,t.p_amt);
        lineOutColor(8,b,WHITE);
        sprintf(b,"UPDATED STOCK: %d",items[k].it_cs);
        lineOutColor(9,b,WHITE);
        sprintf(b,"PURCHASE TOTAL: %d",total);
        lineOutColor(10,b,BROWN);

        more=confirm("Add another product? (Y/N): ");
    }

    frame();
    clearContent();
    lineOutColor(5,"PURCHASE SUMMARY",BROWN);
    sprintf(b,"Purchase No: %d",t.p_no); lineOut(7,b);
    sprintf(b,"Products added: %d",lines); lineOut(8,b);
    sprintf(b,"Total amount: %d",total); lineOutColor(9,b,WHITE);
    pauseScreen();
}
void viewPurchases(void)
{
    int i,start,end,row;
    if(purchaseCount==0) {
        message("No purchase records found.",0);
        return;
    }

    start=0;
    while(start<purchaseCount) {
        frame();
        clearContent();
        lineOutColor(5,"PURCHASE HISTORY",BROWN);

        /* Use fixed cursor positions instead of adjacent %-width fields.
           This prevents DATE and CLIENT, or any other columns, touching. */
        textcolor(LIGHTCYAN);
        gotoxy(BOX_LEFT+2,6);  cprintf("NO");
        gotoxy(BOX_LEFT+8,6);  cprintf("DATE");
        gotoxy(BOX_LEFT+19,6); cprintf("CLIENT");
        gotoxy(BOX_LEFT+31,6); cprintf("ITEM");
        gotoxy(BOX_LEFT+38,6); cprintf("QTY");
        gotoxy(BOX_LEFT+43,6); cprintf("PRICE");
        gotoxy(BOX_LEFT+50,6); cprintf("AMOUNT");

        gotoxy(BOX_LEFT+2,7);
        cprintf("------------------------------------------------------");

        end=start+6;
        if(end>purchaseCount) end=purchaseCount;
        row=9;

        for(i=start;i<end;i++,row++) {
            textcolor(WHITE);
            gotoxy(BOX_LEFT+2,row);  cprintf("%-5d",purchases[i].p_no);
            gotoxy(BOX_LEFT+8,row);  cprintf("%-10.10s",purchases[i].p_date);
            gotoxy(BOX_LEFT+19,row); cprintf("%-10.10s",purchases[i].c_name);
            gotoxy(BOX_LEFT+31,row); cprintf("%-6d",purchases[i].p_code);
            gotoxy(BOX_LEFT+38,row); cprintf("%-4d",purchases[i].p_qty);
            gotoxy(BOX_LEFT+43,row); cprintf("%-6d",purchases[i].p_price);
            gotoxy(BOX_LEFT+50,row); cprintf("%-8d",purchases[i].p_amt);
        }
        pauseScreen();
        start=end;
    }
}
/* Display every product line belonging to a purchase number, six lines per page. */
void showPurchaseLines(int no)
{
    int i,count,start,shown,row;
    char b[80];
    count=0;
    for(i=0;i<purchaseCount;i++) if(purchases[i].p_no==no) count++;
    if(count==0) { message("Purchase number not found.",0); return; }

    start=0;
    while(start<count) {
        frame(); clearContent(); lineOutColor(5,"PURCHASE DETAILS",BROWN);
        sprintf(b,"Purchase No: %d   Product lines: %d",no,count); lineOut(6,b);
        lineOut(7,"CODE  ITEM NAME           QTY   PRICE   AMOUNT");
        lineOut(8,"------------------------------------------------");
        row=9; shown=0;
        for(i=0;i<purchaseCount;i++) {
            if(purchases[i].p_no==no) {
                if(shown>=start && shown<start+6) {
                    int k=itemIndex(purchases[i].p_code);
                    gotoxy(BOX_LEFT+2,row++); textcolor(WHITE);
                    cprintf("%-5d %-20.20s %-5d %-7d %-7d",
                        purchases[i].p_code,(k>=0?items[k].it_name:"[MISSING]"),
                        purchases[i].p_qty,purchases[i].p_price,purchases[i].p_amt);
                }
                shown++;
            }
        }
        sprintf(b,"Date: %.10s  Client: %.20s",purchases[purchaseIndex(no)].p_date,
                purchases[purchaseIndex(no)].c_name);
        lineOut(15,b);
        pauseScreen();
        start+=6;
    }
}

void editPurchase(void)
{
    int no,code,i,row,newQty,newPrice,newAmount;
    int stockIndex,oldQty,adjustedStock;
    char b[80];

    frame(); clearContent(); lineOutColor(5,"EDIT PURCHASE PRODUCT",BROWN);
    lineOut(7,"The purchase's product lines will be shown first.");
    no=readInt("Purchase number: ");
    if(purchaseIndex(no)<0) { message("Purchase number not found.",0); return; }
    showPurchaseLines(no);

    code=readInt("Product code to edit: ");
    row=-1;
    for(i=0;i<purchaseCount;i++)
        if(purchases[i].p_no==no && purchases[i].p_code==code) { row=i; break; }
    if(row<0) { message("Product line not found in this purchase.",0); return; }
    stockIndex=itemIndex(purchases[row].p_code);
    if(stockIndex<0) { message("Linked item is missing.",0); return; }

    frame(); clearContent(); lineOutColor(5,"CURRENT PURCHASE LINE",BROWN);
    sprintf(b,"Purchase number : %d",purchases[row].p_no); lineOut(7,b);
    sprintf(b,"Item            : %s",items[stockIndex].it_name); lineOut(8,b);
    sprintf(b,"Current quantity: %d",purchases[row].p_qty); lineOut(9,b);
    sprintf(b,"Current price   : %d",purchases[row].p_price); lineOut(10,b);
    sprintf(b,"Current amount  : %d",purchases[row].p_amt); lineOut(11,b);
    oldQty=purchases[row].p_qty;
    newQty=readInt("New quantity: ");
    newPrice=readInt("New purchase price: ");
    if(newQty<=0 || newPrice<0) { message("Invalid quantity or price.",0); return; }

    adjustedStock=items[stockIndex].it_cs-oldQty+newQty;
    if(adjustedStock<0) { message("Cannot edit: stock would become negative.",0); return; }
    newAmount=newQty*newPrice;
    frame(); clearContent(); lineOutColor(5,"CONFIRM PURCHASE CHANGE",BROWN);
    sprintf(b,"Purchase: %d  Item: %s",no,items[stockIndex].it_name); lineOut(7,b);
    sprintf(b,"Quantity: %d -> %d",oldQty,newQty); lineOut(8,b);
    sprintf(b,"Price: %d -> %d",purchases[row].p_price,newPrice); lineOut(9,b);
    sprintf(b,"New line amount: %d",newAmount); lineOut(10,b);
    sprintf(b,"Stock after change: %d",adjustedStock); lineOut(11,b);
    if(!confirm("Save these changes? Type Y: ")) { message("Edit cancelled.",1); return; }

    items[stockIndex].it_cs=adjustedStock;
    purchases[row].p_qty=newQty; purchases[row].p_price=newPrice;
    purchases[row].p_amt=newAmount;
    saveItems(); savePurchases(); message("Purchase product line updated.",1);
}

void deletePurchase(void)
{
    int no,i,j,found,stockIndex,choice,code,row;
    char b[80];
    frame(); clearContent(); lineOutColor(5,"DELETE PURCHASE",BROWN);
    lineOut(7,"View the purchase before choosing what to delete.");
    no=readInt("Purchase number: ");
    found=0;
    for(i=0;i<purchaseCount;i++) if(purchases[i].p_no==no) found=1;
    if(!found) { message("Purchase number not found.",0); return; }
    showPurchaseLines(no);

    frame(); clearContent(); lineOutColor(5,"DELETE OPTIONS",BROWN);
    lineOut(7,"1. Delete one product line");
    lineOut(8,"2. Delete the entire purchase");
    lineOut(9,"3. Cancel");
    choice=readInt("Choose an option: ");
    if(choice==3) { message("Deletion cancelled.",1); return; }
    if(choice!=1 && choice!=2) { message("Invalid deletion option.",0); return; }

    if(choice==1) {
        code=readInt("Product code to delete: "); row=-1;
        for(i=0;i<purchaseCount;i++)
            if(purchases[i].p_no==no && purchases[i].p_code==code) { row=i; break; }
        if(row<0) { message("Product line not found in this purchase.",0); return; }
        stockIndex=itemIndex(purchases[row].p_code);
        if(stockIndex<0) { message("Linked item is missing; cannot delete.",0); return; }
        frame(); clearContent(); lineOutColor(5,"CONFIRM LINE DELETION",BROWN);
        sprintf(b,"Purchase number: %d",no); lineOut(7,b);
        sprintf(b,"Item: %s",items[stockIndex].it_name); lineOut(8,b);
        sprintf(b,"Quantity to remove from stock: %d",purchases[row].p_qty); lineOut(9,b);
        if(items[stockIndex].it_cs<purchases[row].p_qty) {
            lineOutColor(11,"Deletion blocked: stock would become negative.",LIGHTRED);
            pauseScreen(); return;
        }
        if(!confirm("Delete this line? Type Y: ")) { message("Deletion cancelled.",1); return; }
        items[stockIndex].it_cs-=purchases[row].p_qty;
        for(j=row;j<purchaseCount-1;j++) purchases[j]=purchases[j+1];
        purchaseCount--; saveItems(); savePurchases();
        message("One purchase product line deleted.",1);
        return;
    }

    /* Validate every line before changing stock or deleting anything. */
    for(i=0;i<purchaseCount;i++) if(purchases[i].p_no==no) {
        stockIndex=itemIndex(purchases[i].p_code);
        if(stockIndex<0) { message("A linked item is missing; cannot delete.",0); return; }
        if(items[stockIndex].it_cs<purchases[i].p_qty) {
            message("Deletion blocked: stock would become negative.",0); return;
        }
    }
    sprintf(b,"Delete purchase %d and ALL its product lines?",no);
    if(!confirm(b)) { message("Deletion cancelled.",1); return; }
    i=0;
    while(i<purchaseCount) {
        if(purchases[i].p_no==no) {
            stockIndex=itemIndex(purchases[i].p_code);
            items[stockIndex].it_cs-=purchases[i].p_qty;
            for(j=i;j<purchaseCount-1;j++) purchases[j]=purchases[j+1];
            purchaseCount--;
        } else i++;
    }
    saveItems(); savePurchases();
    sprintf(b,"Purchase %d and all its lines deleted.",no); message(b,1);
}
void purchaseMenu(void)
{
    int c;
    do {
        frame(); clearContent(); lineOutColor(6,"PURCHASE MANAGEMENT",BROWN);
        lineOut(8,"1. Record Purchase"); lineOut(9,"2. Edit Purchase");
        lineOut(10,"3. Delete Purchase"); lineOut(11,"4. View Purchases");
        lineOut(12,"5. Back");
        c=readInt("Purchase menu choice: ");
        switch(c) {
            case 1:addPurchase();break; case 2:editPurchase();break;
            case 3:deletePurchase();break; case 4:viewPurchases();break;
            case 5:break; default:message("Invalid choice.",0);
        }
    } while(c!=5);
}

/* ---------- Sales functions ---------- */
void addSale(void)
{
    struct Sales t;
    int k,more,lines,i,duplicate,subtotal,totalTax,totalAmount;
    char b[80];

    if(salesCount>=MAXREC) { message("Sales file is full.",0); return; }
    t.b_no=nextBillNumber++;
    saveNextNumber(BILLNOFILE,nextBillNumber);

    /* Draw a fresh bill-entry screen instead of leaving the menu behind the prompt. */
    frame(); clearContent();
    lineOutColor(5,"CREATE MULTI-PRODUCT SALES BILL",BROWN);
    sprintf(b,"Bill No: %d",t.b_no);
    lineOut(7,b);
    do {
        readText("Bill date (DD-MM-YYYY): ",t.b_date,11);
        if(!validDate(t.b_date)) message("Invalid date. Use a real DD-MM-YYYY date.",0);
    } while(!validDate(t.b_date));

    lines=0; subtotal=0; totalTax=0; totalAmount=0; more=1;
    while(more==1 && salesCount<MAXREC) {
        frame(); clearContent();
        lineOutColor(5,"CREATE MULTI-PRODUCT SALES BILL",BROWN);
        sprintf(b,"Bill No: %d  Date: %.10s",t.b_no,t.b_date);
        lineOutColor(6,b,WHITE);
        sprintf(b,"Product line %d",lines+1); lineOutColor(7,b,BROWN);

        t.p_code=readInt("Product/item code (0 to finish): ");
        if(t.p_code==0) {
            if(lines==0) { message("No products added; bill cancelled.",0); return; }
            break;
        }
        k=itemIndex(t.p_code);
        if(k<0) {
            message("Item code does not exist.",0);
            if(lines==0) return;
            more=confirm("Add another product? Type Y: ");
            continue;
        }
        duplicate=0;
        for(i=0;i<salesCount;i++)
            if(sales[i].b_no==t.b_no && sales[i].p_code==t.p_code) duplicate=1;
        if(duplicate) {
            message("This item is already on this bill.",0);
            more=confirm("Add another product? Type Y: ");
            continue;
        }

        sprintf(b,"ITEM: %.20s",items[k].it_name); lineOut(8,b);
        sprintf(b,"AVAILABLE STOCK: %d",items[k].it_cs); lineOut(9,b);
        sprintf(b,"UNIT RATE: %d",items[k].it_price); lineOut(10,b);
        t.b_qty=readInt("Quantity sold: ");
        if(t.b_qty<=0) {
            message("Quantity must be positive.",0);
            more=confirm("Try another product? Type Y: ");
            continue;
        }
        if(t.b_qty>items[k].it_cs) {
            message("Insufficient stock for this item.",0);
            more=confirm("Try another product? Type Y: ");
            continue;
        }
        if(salesCount>=MAXREC) { message("Sales file is full.",0); break; }

        t.b_rate=items[k].it_price;
        t.b_tax=0;
        t.b_amt=t.b_qty*t.b_rate;
        items[k].it_cs-=t.b_qty;
        sales[salesCount++]=t;
        recalculateBillTax(t.b_no);
        lines++;
        subtotal+=t.b_qty*t.b_rate;
        totalTax+=t.b_tax;
        totalAmount+=t.b_amt;
        saveItems(); saveSales();
        /* Refresh totals after recalculating tax on the complete bill subtotal. */
        totalTax=0; totalAmount=0;
        for(i=0;i<salesCount;i++) if(sales[i].b_no==t.b_no) {
            totalTax+=sales[i].b_tax; totalAmount+=sales[i].b_amt;
        }
        frame(); clearContent(); lineOutColor(5,"PRODUCT ADDED TO BILL",BROWN);
        sprintf(b,"Item: %.20s",items[k].it_name); lineOut(7,b);
        sprintf(b,"Quantity: %d  Line total: %d",t.b_qty,t.b_amt); lineOut(8,b);
        sprintf(b,"Stock remaining: %d",items[k].it_cs); lineOut(9,b);
        sprintf(b,"Current bill total: %d",totalAmount); lineOut(10,b);
        more=confirm("Add another product? Type Y: ");
    }

    if(lines==0) {
        /* Count saved lines for this bill in case the user finished with code 0. */
        for(i=0;i<salesCount;i++) if(sales[i].b_no==t.b_no) lines++;
    }
    /* Recalculate from saved bill lines to ensure the summary is accurate. */
    subtotal=0; totalTax=0; totalAmount=0; lines=0;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==t.b_no) {
        subtotal+=sales[i].b_qty*sales[i].b_rate;
        totalTax+=sales[i].b_tax;
        totalAmount+=sales[i].b_amt;
        lines++;
    }
    frame(); clearContent(); lineOutColor(5,"SALES BILL GENERATED",BROWN);
    sprintf(b,"Bill number : %d",t.b_no); lineOut(7,b);
    sprintf(b,"Products    : %d",lines); lineOut(8,b);
    sprintf(b,"Subtotal    : %d",subtotal); lineOut(9,b);
    sprintf(b,"Tax (%d%%)    : %d",TAX_RATE,totalTax); lineOut(10,b);
    sprintf(b,"Total amount: %d",totalAmount); lineOut(11,b);
    pauseScreen();
}
void viewSales(void)
{
    int i,start,end,row;
    if(salesCount==0) { message("No sales records found.",0); return; }
    start=0;
    while(start<salesCount) {
        frame(); clearContent(); lineOutColor(5,"SALES HISTORY",BROWN);
        lineOut(6,"BILL DATE         ITEM   QTY   RATE   TAX   AMOUNT");
        lineOut(7,"--------------------------------------------------");
        end=start+6; if(end>salesCount) end=salesCount; row=8;
        for(i=start;i<end;i++,row++) {
            gotoxy(BOX_LEFT+2,row); textcolor(WHITE);
            cprintf("%-4d %-11.11s %-6d %-5d %-6d %-5d %-7d",
                sales[i].b_no,sales[i].b_date,sales[i].p_code,sales[i].b_qty,
                sales[i].b_rate,sales[i].b_tax,sales[i].b_amt);
        }
        pauseScreen(); start=end;
    }
}
void showSaleDetails(int k, char *heading)
{
    char b[80];
    int ik=itemIndex(sales[k].p_code);
    frame(); clearContent(); lineOutColor(5,heading,BROWN);
    sprintf(b,"Bill number : %d",sales[k].b_no); lineOut(7,b);
    sprintf(b,"Bill date   : %s",sales[k].b_date); lineOut(8,b);
    if(ik>=0) sprintf(b,"Item        : %s (code %d)",items[ik].it_name,sales[k].p_code);
    else sprintf(b,"Item code   : %d (linked item missing)",sales[k].p_code);
    lineOut(9,b);
    sprintf(b,"Quantity    : %d",sales[k].b_qty); lineOut(10,b);
    sprintf(b,"Rate        : %d",sales[k].b_rate); lineOut(11,b);
    sprintf(b,"Tax         : %d",sales[k].b_tax); lineOut(12,b);
    sprintf(b,"Total       : %d",sales[k].b_amt); lineOut(13,b);
}

/* Display every product line belonging to one sales bill. */
void showSalesLines(int no)
{
    int i,count,start,shown,row,k;
    char b[80];
    count=0;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==no) count++;
    if(count==0) { message("Sales bill not found.",0); return; }

    start=0;
    while(start<count) {
        frame(); clearContent(); lineOutColor(5,"SALES BILL DETAILS",BROWN);
        sprintf(b,"Bill No: %d   Product lines: %d",no,count); lineOut(6,b);
        lineOut(7,"CODE  ITEM NAME           QTY   RATE    TAX   TOTAL");
        lineOut(8,"----------------------------------------------------");
        row=9; shown=0;
        for(i=0;i<salesCount;i++) if(sales[i].b_no==no) {
            if(shown>=start && shown<start+6) {
                k=itemIndex(sales[i].p_code);
                gotoxy(BOX_LEFT+2,row++); textcolor(WHITE);
                cprintf("%-5d %-20.20s %-5d %-7d %-5d %-7d",
                    sales[i].p_code,(k>=0?items[k].it_name:"[MISSING]"),
                    sales[i].b_qty,sales[i].b_rate,sales[i].b_tax,sales[i].b_amt);
            }
            shown++;
        }
        pauseScreen(); start+=6;
    }
}

void editSale(void)
{
    int no,k,oldk,newk,code,qty,oldQty,i;
    char date[11],b[80];
    frame(); clearContent(); lineOutColor(6,"EDIT SALES BILL PRODUCT",BROWN);
    no=readInt("Bill number: ");
    if(saleIndex(no)<0) { message("Sales bill not found.",0); return; }
    showSalesLines(no);
    code=readInt("Product code to edit: ");
    k=-1;
    for(i=0;i<salesCount;i++)
        if(sales[i].b_no==no && sales[i].p_code==code) { k=i; break; }
    if(k<0) { message("Product line not found on this bill.",0); return; }
    oldk=itemIndex(sales[k].p_code);
    if(oldk<0) { message("Linked item is missing.",0); return; }
    showSaleDetails(k,"CURRENT SALES BILL PRODUCT");
    pauseScreen();

    do {
        readText("New bill date (Enter to keep current): ",date,11);
        if(date[0]=='\0') strcpy(date,sales[k].b_date);
        else if(!validDate(date)) message("Invalid date. Use DD-MM-YYYY.",0);
    } while(!validDate(date));
    code=readInt("New product code (0 to keep current): ");
    if(code==0) code=sales[k].p_code;
    qty=readInt("New quantity: ");
    newk=itemIndex(code);
    if(newk<0 || qty<=0) { message("Invalid item or quantity.",0); return; }
    for(i=0;i<salesCount;i++)
        if(i!=k && sales[i].b_no==no && sales[i].p_code==code) {
            message("That item already exists on this bill.",0); return;
        }

    oldQty=sales[k].b_qty;
    items[oldk].it_cs+=oldQty;
    if(qty>items[newk].it_cs) {
        items[oldk].it_cs-=oldQty;
        message("Insufficient stock for revised bill.",0); return;
    }
    frame(); clearContent(); lineOutColor(5,"CONFIRM SALES LINE CHANGE",BROWN);
    sprintf(b,"Bill number: %d",no); lineOut(7,b);
    sprintf(b,"Date: %.10s",date); lineOut(8,b);
    sprintf(b,"Product: %s",items[newk].it_name); lineOut(9,b);
    sprintf(b,"Quantity: %d -> %d",oldQty,qty); lineOut(10,b);
    sprintf(b,"New subtotal: %d",qty*items[newk].it_price); lineOut(11,b);
    lineOut(12,"Bill tax will be recalculated on the full subtotal.");
    sprintf(b,"New line subtotal: %ld",(long)qty*items[newk].it_price); lineOut(13,b);
    if(!confirm("Save changes? Type Y: ")) {
        items[oldk].it_cs-=oldQty;
        message("Edit cancelled.",1); return;
    }
    items[newk].it_cs-=qty;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==no) strcpy(sales[i].b_date,date);
    sales[k].p_code=code; sales[k].b_qty=qty;
    sales[k].b_rate=items[newk].it_price;
    sales[k].b_tax=0;
    sales[k].b_amt=qty*sales[k].b_rate;
    recalculateBillTax(no);
    saveItems(); saveSales(); message("Sales bill product line updated.",1);
}

void deleteSale(void)
{
    int no,i,j,found,ik,choice,code,row,units,lines;
    long subtotal,tax,total;
    char b[80];
    frame(); clearContent(); lineOutColor(6,"DELETE SALES BILL",BROWN);
    no=readInt("Bill number: ");
    found=0;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==no) found=1;
    if(!found) { message("Sales bill not found.",0); return; }

    showSalesLines(no);
    lines=0; units=0; subtotal=0; tax=0; total=0;
    for(i=0;i<salesCount;i++) if(sales[i].b_no==no) {
        lines++; units+=sales[i].b_qty;
        subtotal+=(long)sales[i].b_qty*sales[i].b_rate;
        tax+=sales[i].b_tax; total+=sales[i].b_amt;
    }
    frame(); clearContent(); lineOutColor(5,"DELETE OPTIONS",BROWN);
    sprintf(b,"Bill No: %d  Date: %.10s",no,sales[saleIndex(no)].b_date); lineOut(6,b);
    sprintf(b,"Products: %d  Units: %d",lines,units); lineOut(7,b);
    sprintf(b,"Subtotal: %ld  Tax: %ld",subtotal,tax); lineOut(8,b);
    sprintf(b,"Bill total: %ld",total); lineOut(9,b);
    lineOut(11,"1. Delete one product line");
    lineOut(12,"2. Delete the entire bill");
    lineOut(13,"3. Cancel");
    choice=readInt("Choose an option: ");
    if(choice==3) { message("Deletion cancelled.",1); return; }
    if(choice!=1 && choice!=2) { message("Invalid deletion option.",0); return; }

    if(choice==1) {
        code=readInt("Product code to delete: "); row=-1;
        for(i=0;i<salesCount;i++)
            if(sales[i].b_no==no && sales[i].p_code==code) { row=i; break; }
        if(row<0) { message("Product line not found on this bill.",0); return; }
        ik=itemIndex(sales[row].p_code);
        if(ik<0) { message("Linked item is missing; cannot delete.",0); return; }
        frame(); clearContent(); lineOutColor(5,"CONFIRM SALES LINE DELETION",BROWN);
        sprintf(b,"Bill number: %d  Date: %.10s",no,sales[row].b_date); lineOut(6,b);
        sprintf(b,"Product: %.20s",items[ik].it_name); lineOut(7,b);
        sprintf(b,"Code: %d  Quantity: %d",sales[row].p_code,sales[row].b_qty); lineOut(8,b);
        sprintf(b,"Unit rate: %d  Tax: %d",sales[row].b_rate,sales[row].b_tax); lineOut(9,b);
        sprintf(b,"Line total: %d",sales[row].b_amt); lineOut(10,b);
        sprintf(b,"Stock after return: %d",items[ik].it_cs+sales[row].b_qty); lineOut(11,b);
        if(!confirm("Type Y to delete this line (Y/N): ")) {
            message("Deletion cancelled; bill unchanged.",1); return;
        }
        items[ik].it_cs+=sales[row].b_qty;
        for(j=row;j<salesCount-1;j++) sales[j]=sales[j+1];
        salesCount--;
        recalculateBillTax(no);
        saveItems(); saveSales();
        message("Sales product line deleted; stock restored.",1);
        return;
    }

    for(i=0;i<salesCount;i++) if(sales[i].b_no==no && itemIndex(sales[i].p_code)<0) {
        message("A linked item is missing; bill was not deleted.",0); return;
    }
    frame(); clearContent(); lineOutColor(5,"CONFIRM ENTIRE BILL DELETION",BROWN);
    sprintf(b,"Bill number: %d  Date: %.10s",no,sales[saleIndex(no)].b_date); lineOut(6,b);
    sprintf(b,"Product lines: %d  Total units: %d",lines,units); lineOut(7,b);
    sprintf(b,"Subtotal: %ld",subtotal); lineOut(8,b);
    sprintf(b,"Tax: %ld  Total: %ld",tax,total); lineOut(9,b);
    lineOut(11,"All listed product lines will be removed.");
    if(!confirm("Type Y to delete entire bill (Y/N): ")) {
        message("Deletion cancelled; bill unchanged.",1); return;
    }
    i=0;
    while(i<salesCount) {
        if(sales[i].b_no==no) {
            ik=itemIndex(sales[i].p_code);
            items[ik].it_cs+=sales[i].b_qty;
            for(j=i;j<salesCount-1;j++) sales[j]=sales[j+1];
            salesCount--;
        } else i++;
    }
    saveItems(); saveSales();
    sprintf(b,"Bill %d deleted; all stock restored.",no);
    message(b,1);
}
void salesMenu(void)
{
    int c;
    do {
        frame(); clearContent(); lineOutColor(6,"SALES MANAGEMENT",BROWN);
        lineOut(8,"1. Create Sales Bill"); lineOut(9,"2. Edit Sales Bill");
        lineOut(10,"3. Delete Sales Bill"); lineOut(11,"4. View Sales");
        lineOut(12,"5. Back");
        c=readInt("Sales menu choice: ");
        switch(c) {
            case 1:addSale();break; case 2:editSale();break;
            case 3:deleteSale();break; case 4:viewSales();break;
            case 5:break; default:message("Invalid choice.",0);
        }
    } while(c!=5);
}

/* ---------- Reports ---------- */
void lowStockReport(void)
{
    int threshold,i,row,found;
    frame(); clearContent(); lineOutColor(6,"LOW STOCK REPORT",BROWN);
    threshold=readInt("Enter threshold (example 5): ");
    if(threshold<0) { message("Threshold cannot be negative.",0); return; }
    frame(); clearContent(); lineOutColor(5,"LOW STOCK ITEMS",BROWN);
    lineOut(6,"CODE  ITEM NAME           CURRENT STOCK");
    lineOut(7,"---------------------------------------");
    row=8; found=0;
    for(i=0;i<itemCount && row<=14;i++) {
        if(items[i].it_cs<=threshold) {
            gotoxy(BOX_LEFT+2,row); textcolor(YELLOW);
            cprintf("%-5d %-20.20s %-5d",items[i].it_code,items[i].it_name,items[i].it_cs);
            row++; found=1;
        }
    }
    if(!found) lineOut(9,"No items are at or below this threshold.");
    pauseScreen();
}
void inventorySummary(void)
{
    int i,units,low,empty; long value; char b[80];
    units=0; low=0; empty=0; value=0L;
    for(i=0;i<itemCount;i++) {
        units+=items[i].it_cs;
        value+=(long)items[i].it_cs*items[i].it_price;
        if(items[i].it_cs<=5) low++;
        if(items[i].it_cs==0) empty++;
    }
    frame(); clearContent(); lineOutColor(6,"INVENTORY SUMMARY",BROWN);
    sprintf(b,"Number of different items: %d",itemCount); lineOut(8,b);
    sprintf(b,"Total stock units       : %d",units); lineOut(9,b);
    sprintf(b,"Low-stock items (<=5)   : %d",low); lineOut(10,b);
    sprintf(b,"Out-of-stock items      : %d",empty); lineOut(11,b);
    sprintf(b,"Inventory value         : %ld",value); lineOut(12,b);
    pauseScreen();
}

/* ---------- Main menu ---------- */
void mainMenu(void)
{
    int c;
    do {
        frame(); clearContent(); lineOutColor(6,"MAIN MENU",BROWN);
        lineOut(8,"1. Item Management");
        lineOut(9,"2. Purchase Management");
        lineOut(10,"3. Sales Management");
        lineOut(11,"4. Low Stock Report");
        lineOut(12,"5. Inventory Summary");
        lineOut(13,"6. Exit");
        c=readInt("Enter your choice: ");
        switch(c) {
            case 1:itemMenu();break;
            case 2:purchaseMenu();break;
            case 3:salesMenu();break;
            case 4:lowStockReport();break;
            case 5:inventorySummary();break;
            case 6:
                if(!confirm("Exit program? Type Y to confirm: ")) c=0;
                break;
            default:message("Invalid choice. Select 1 to 6.",0);
        }
    } while(c!=6);
}
void main(void)
{
    loadAll();
    mainMenu();
    textbackground(BLACK); textcolor(WHITE); clrscr();
    cprintf("Inventory Management System closed.");
}
