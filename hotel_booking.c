#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROOMS 10
#define DATA_FILE "bookings.dat"

typedef struct {
    int bookingID;
    char guestName[60];
    char phone[20];
    char idNumber[20];
    int roomNumber;
    int nights;
    double totalAmount;
    double amountPaid;
    int active;
} Booking;

typedef struct {
    int roomNumber;
    const char *type;
    double pricePerNight;
} Room;

const Room rooms[MAX_ROOMS] = {
    {101, "Single", 1500.00},
    {102, "Single", 1500.00},
    {103, "Double", 2500.00},
    {104, "Double", 2500.00},
    {105, "Twin", 3000.00},
    {106, "Twin", 3000.00},
    {107, "Deluxe", 4500.00},
    {108, "Deluxe", 4500.00},
    {109, "Executive", 6000.00},
    {110, "Executive", 6000.00}
};

void clearInput(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        /* discard remaining input */
    }
}

void readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

int readInt(const char *prompt) {
    int value;
    printf("%s", prompt);
    while (scanf("%d", &value) != 1) {
        printf("Please enter a valid whole number: ");
        clearInput();
    }
    clearInput();
    return value;
}

double readDouble(const char *prompt) {
    double value;
    printf("%s", prompt);
    while (scanf("%lf", &value) != 1 || value < 0) {
        printf("Please enter a valid amount (0 or greater): ");
        clearInput();
    }
    clearInput();
    return value;
}

int roomExists(int roomNumber) {
    int i;
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].roomNumber == roomNumber) {
            return 1;
        }
    }
    return 0;
}

double roomPrice(int roomNumber) {
    int i;
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].roomNumber == roomNumber) {
            return rooms[i].pricePerNight;
        }
    }
    return 0.0;
}

const char *roomType(int roomNumber) {
    int i;
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].roomNumber == roomNumber) {
            return rooms[i].type;
        }
    }
    return "Unknown";
}

int roomIsOccupied(int roomNumber) {
    FILE *file = fopen(DATA_FILE, "rb");
    Booking booking;

    if (file == NULL) {
        return 0;
    }

    while (fread(&booking, sizeof(Booking), 1, file) == 1) {
        if (booking.active && booking.roomNumber == roomNumber) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int nextBookingID(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    Booking booking;
    int highestID = 1000;

    if (file == NULL) {
        return highestID + 1;
    }

    while (fread(&booking, sizeof(Booking), 1, file) == 1) {
        if (booking.bookingID > highestID) {
            highestID = booking.bookingID;
        }
    }

    fclose(file);
    return highestID + 1;
}

void displayRooms(void) {
    int i;

    printf("\n================ ROOM LIST ================\n");
    printf("%-12s %-15s %-18s %-12s\n",
           "Room No.", "Room Type", "Price/Night (KSh)", "Status");

    for (i = 0; i < MAX_ROOMS; i++) {
        printf("%-12d %-15s %-18.2f %-12s\n",
               rooms[i].roomNumber,
               rooms[i].type,
               rooms[i].pricePerNight,
               roomIsOccupied(rooms[i].roomNumber) ? "Occupied" : "Available");
    }
}

void registerBooking(void) {
    Booking booking;
    FILE *file;
    int roomNumber;

    printf("\n============== NEW ROOM BOOKING ==============\n");

    readLine("Guest full name: ", booking.guestName, sizeof(booking.guestName));
    readLine("Phone number: ", booking.phone, sizeof(booking.phone));
    readLine("National ID/Passport number: ", booking.idNumber, sizeof(booking.idNumber));

    if (strlen(booking.guestName) == 0 || strlen(booking.phone) == 0 ||
        strlen(booking.idNumber) == 0) {
        printf("\nAll guest details are required. Booking was not saved.\n");
        return;
    }

    displayRooms();
    roomNumber = readInt("\nEnter the room number you want to book: ");

    if (!roomExists(roomNumber)) {
        printf("\nThat room number does not exist.\n");
        return;
    }

    if (roomIsOccupied(roomNumber)) {
        printf("\nSorry, that room is already occupied.\n");
        return;
    }

    booking.nights = readInt("Number of nights: ");
    if (booking.nights <= 0) {
        printf("\nNumber of nights must be at least 1.\n");
        return;
    }

    booking.bookingID = nextBookingID();
    booking.roomNumber = roomNumber;
    booking.totalAmount = roomPrice(roomNumber) * booking.nights;
    booking.amountPaid = 0.0;
    booking.active = 1;

    file = fopen(DATA_FILE, "ab");
    if (file == NULL) {
        printf("\nError: Could not open the booking data file.\n");
        return;
    }

    if (fwrite(&booking, sizeof(Booking), 1, file) != 1) {
        printf("\nError: Booking could not be saved.\n");
        fclose(file);
        return;
    }

    fclose(file);

    printf("\nBooking successfully created!\n");
    printf("---------------------------------------------\n");
    printf("Booking ID : %d\n", booking.bookingID);
    printf("Guest      : %s\n", booking.guestName);
    printf("Room       : %d (%s)\n", booking.roomNumber, roomType(roomNumber));
    printf("Nights     : %d\n", booking.nights);
    printf("Total      : KSh %.2f\n", booking.totalAmount);
    printf("---------------------------------------------\n");
}

void viewAllBookings(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    Booking booking;
    int found = 0;

    printf("\n================ ALL ACTIVE BOOKINGS ================\n");

    if (file == NULL) {
        printf("No booking records have been created yet.\n");
        return;
    }

    printf("%-10s %-22s %-12s %-8s %-13s %-13s\n",
           "ID", "Guest", "Room", "Nights", "Total(KSh)", "Paid(KSh)");

    while (fread(&booking, sizeof(Booking), 1, file) == 1) {
        if (booking.active) {
            printf("%-10d %-22.22s %-12d %-8d %-13.2f %-13.2f\n",
                   booking.bookingID, booking.guestName, booking.roomNumber,
                   booking.nights, booking.totalAmount, booking.amountPaid);
            found = 1;
        }
    }

    if (!found) {
        printf("There are no active bookings.\n");
    }

    fclose(file);
}

void searchBooking(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    Booking booking;
    int searchID;
    int found = 0;

    if (file == NULL) {
        printf("\nNo booking records found.\n");
        return;
    }

    searchID = readInt("\nEnter booking ID to search: ");

    while (fread(&booking, sizeof(Booking), 1, file) == 1) {
        if (booking.bookingID == searchID && booking.active) {
            printf("\n--------------- BOOKING DETAILS ---------------\n");
            printf("Booking ID : %d\n", booking.bookingID);
            printf("Guest      : %s\n", booking.guestName);
            printf("Phone      : %s\n", booking.phone);
            printf("ID/Passport: %s\n", booking.idNumber);
            printf("Room       : %d (%s)\n", booking.roomNumber,
                   roomType(booking.roomNumber));
            printf("Nights     : %d\n", booking.nights);
            printf("Total      : KSh %.2f\n", booking.totalAmount);
            printf("Paid       : KSh %.2f\n", booking.amountPaid);
            printf("Balance    : KSh %.2f\n",
                   booking.totalAmount - booking.amountPaid);
            printf("------------------------------------------------\n");
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nNo active booking found with that ID.\n");
    }

    fclose(file);
}

void recordPayment(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    FILE *temp;
    Booking booking;
    int searchID;
    int found = 0;
    double payment;

    if (file == NULL) {
        printf("\nNo booking records found.\n");
        return;
    }

    searchID = readInt("\nEnter booking ID for payment: ");
    temp = fopen("temp.dat", "wb");

    if (temp == NULL) {
        printf("\nCould not create temporary data file.\n");
        fclose(file);
        return;
    }

    while (fread(&booking, sizeof(Booking), 1, file) == 1) {
        if (booking.bookingID == searchID && booking.active) {
            double balance = booking.totalAmount - booking.amountPaid;
            printf("Guest: %s\n", booking.guestName);
            printf("Outstanding balance: KSh %.2f\n", balance);

            if (balance <= 0.0) {
                printf("This booking is already fully paid.\n");
            } else {
                payment = readDouble("Enter amount received (KSh): ");
                if (payment <= 0.0 || payment > balance) {
                    printf("Payment must be greater than zero and not exceed the balance.\n");
                } else {
                    booking.amountPaid += payment;
                    printf("Payment recorded successfully.\n");
                    printf("New balance: KSh %.2f\n",
                           booking.totalAmount - booking.amountPaid);
                }
            }
            found = 1;
        }

        fwrite(&booking, sizeof(Booking), 1, temp);
    }

    fclose(file);
    fclose(temp);

    if (remove(DATA_FILE) != 0 || rename("temp.dat", DATA_FILE) != 0) {
        printf("Warning: Could not update the booking file. Check temp.dat.\n");
        return;
    }

    if (!found) {
        printf("\nNo active booking found with that ID.\n");
    }
}

void cancelBooking(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    FILE *temp;
    Booking booking;
    int searchID;
    int found = 0;

    if (file == NULL) {
        printf("\nNo booking records found.\n");
        return;
    }

    searchID = readInt("\nEnter booking ID to cancel: ");
    temp = fopen("temp.dat", "wb");

    if (temp == NULL) {
        printf("\nCould not create temporary data file.\n");
        fclose(file);
        return;
    }

    while (fread(&booking, sizeof(Booking), 1, file) == 1) {
        if (booking.bookingID == searchID && booking.active) {
            booking.active = 0;
            found = 1;
            printf("Booking %d for %s has been cancelled.\n",
                   booking.bookingID, booking.guestName);
        }
        fwrite(&booking, sizeof(Booking), 1, temp);
    }

    fclose(file);
    fclose(temp);

    if (remove(DATA_FILE) != 0 || rename("temp.dat", DATA_FILE) != 0) {
        printf("Warning: Could not update the booking file. Check temp.dat.\n");
        return;
    }

    if (!found) {
        printf("\nNo active booking found with that ID.\n");
    }
}

void printReceipt(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    Booking booking;
    int searchID;
    int found = 0;

    if (file == NULL) {
        printf("\nNo booking records found.\n");
        return;
    }

    searchID = readInt("\nEnter booking ID for receipt: ");

    while (fread(&booking, sizeof(Booking), 1, file) == 1) {
        if (booking.bookingID == searchID && booking.active) {
            printf("\n============================================\n");
            printf("             NEXORA GRAND HOTEL             \n");
            printf("                 PAYMENT RECEIPT            \n");
            printf("============================================\n");
            printf("Booking ID : %d\n", booking.bookingID);
            printf("Guest      : %s\n", booking.guestName);
            printf("Phone      : %s\n", booking.phone);
            printf("Room       : %d (%s)\n", booking.roomNumber,
                   roomType(booking.roomNumber));
            printf("Nights     : %d\n", booking.nights);
            printf("--------------------------------------------\n");
            printf("Total      : KSh %.2f\n", booking.totalAmount);
            printf("Paid       : KSh %.2f\n", booking.amountPaid);
            printf("Balance    : KSh %.2f\n",
                   booking.totalAmount - booking.amountPaid);
            printf("--------------------------------------------\n");
            printf("Thank you for choosing Nexora Grand Hotel!\n");
            printf("============================================\n");
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nNo active booking found with that ID.\n");
    }

    fclose(file);
}

void showMenu(void) {
    printf("\n================================================\n");
    printf("          NEXORA GRAND HOTEL SYSTEM             \n");
    printf("             HOTEL MANAGEMENT                   \n");
    printf("================================================\n");
    printf("1. Display rooms and availability\n");
    printf("2. Register a new booking\n");
    printf("3. View all active bookings\n");
    printf("4. Search for a booking\n");
    printf("5. Record a guest payment\n");
    printf("6. Cancel a booking\n");
    printf("7. Print a booking receipt\n");
    printf("0. Exit\n");
    printf("================================================\n");
}

int main(void) {
    int choice;

    do {
        showMenu();
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                displayRooms();
                break;
            case 2:
                registerBooking();
                break;
            case 3:
                viewAllBookings();
                break;
            case 4:
                searchBooking();
                break;
            case 5:
                recordPayment();
                break;
            case 6:
                cancelBooking();
                break;
            case 7:
                printReceipt();
                break;
            case 0:
                printf("\nThank you for using Nexora Grand Hotel System. Goodbye!\n");
                break;
            default:
                printf("\nInvalid choice. Please select a menu option from 0 to 7.\n");
        }

    } while (choice != 0);

    return 0;
}
