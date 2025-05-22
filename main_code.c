#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 500
#define MAX_ROOMS 45

struct Student {
    int rollNo;
    char name[50];
    int examRoom;
};

struct SwapHistory {
    int rollFrom;
    int roomFrom;
    int rollTo;
    int roomTo;
};

struct Student students[MAX_STUDENTS];
int numStudents = 0;
int maxPerRoom = 20;

struct SwapHistory lastSwap;
int undoAvailable = 0;

// Count how many students are in a room
int countStudentsInRoom(int room) {
    int count = 0;
    for (int i = 0; i < numStudents; i++) {
        if (students[i].examRoom == room)
            count++;
    }
    return count;
}

// Read student data and auto-assign rooms
void readDataAndAssignRooms() {
    FILE *fp = fopen("C:\\Users\\visma\\Desktop\\C Programming\\JOB Project\\data.csv", "r");
    if (!fp) {
        printf("Could not open CSV file.\n");
        exit(1);
    }

    char line[200];
    fgets(line, sizeof(line), fp); // Skip header

    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < MAX_STUDENTS) {
        char *token = strtok(line, ",");
        students[index].rollNo = atoi(token);

        token = strtok(NULL, ",");
        if (token != NULL)
            strcpy(students[index].name, token);

        students[index].examRoom = (index / maxPerRoom) + 1;
        index++;
    }

    numStudents = index;
    fclose(fp);
}

// Display student info by roll number
void searchByRollNumber() {
    int roll;
    printf("Enter roll number: ");
    scanf("%d", &roll);

    for (int i = 0; i < numStudents; i++) {
        if (students[i].rollNo == roll) {
            printf("Student: %s (Roll No: %d)\n", students[i].name, students[i].rollNo);
            printf("Assigned Room: %d\n", students[i].examRoom);
            return;
        }
    }
    printf("Student not found.\n");
}

// Find index of student by roll number
int findStudentIndex(int roll) {
    for (int i = 0; i < numStudents; i++) {
        if (students[i].rollNo == roll)
            return i;
    }
    return -1;
}

// Helper function: check duplicates for a roll number and print warning with names
int checkDuplicateRoll(int roll) {
    int count = 0;
    for (int i = 0; i < numStudents; i++) {
        if (students[i].rollNo == roll)
            count++;
    }
    if (count > 1) {
        printf("Warning: Duplicate roll number %d found for students:\n", roll);
        for (int i = 0; i < numStudents; i++) {
            if (students[i].rollNo == roll)
                printf(" - %s\n", students[i].name);
        }
        return 1; // duplicates found
    }
    return 0; // no duplicates
}

// Replace student A with B (swap room numbers)
void replaceStudentInRoom() {
    int rollFrom, rollTo;
    printf("Enter roll number of student to move: ");
    scanf("%d", &rollFrom);
    printf("Enter roll number of student currently in target room (to be replaced): ");
    scanf("%d", &rollTo);

    // Check duplicates for both roll numbers before swapping
    if (checkDuplicateRoll(rollFrom) || checkDuplicateRoll(rollTo)) {
        printf("Cannot perform swap due to duplicate roll numbers.\n");
        return;
    }

    int fromIndex = findStudentIndex(rollFrom);
    int toIndex = findStudentIndex(rollTo);

    if (fromIndex == -1 || toIndex == -1) {
        printf("One or both roll numbers not found.\n");
        return;
    }

    int roomFrom = students[fromIndex].examRoom;
    int roomTo = students[toIndex].examRoom;

    if (roomFrom == roomTo) {
        printf("Both students are already in Room %d. No action taken.\n", roomFrom);
        return;
    }

    // Save swap for undo
    lastSwap.rollFrom = rollFrom;
    lastSwap.roomFrom = roomFrom;
    lastSwap.rollTo = rollTo;
    lastSwap.roomTo = roomTo;
    undoAvailable = 1;

    // Swap rooms
    students[fromIndex].examRoom = roomTo;
    students[toIndex].examRoom = roomFrom;

    // Log the change
    FILE *log = fopen("C:\\Users\\visma\\Desktop\\C Programming\\JOB Project\\updated_allotment.csv", "a");
    if (log) {
        fprintf(log, "Swapped: %s (Roll %d) [Room %d] with %s (Roll %d) [Room %d]\n",
                students[fromIndex].name, rollFrom, roomFrom,
                students[toIndex].name, rollTo, roomTo);
        fclose(log);
    }

    printf("Swap completed successfully.\n");
}

// Undo the last replacement
void undoLastReplacement() {
    if (!undoAvailable) {
        printf("No replacement to undo.\n");
        return;
    }

    int fromIndex = findStudentIndex(lastSwap.rollFrom);
    int toIndex = findStudentIndex(lastSwap.rollTo);

    if (fromIndex == -1 || toIndex == -1) {
        printf("Undo failed: One or both students not found.\n");
        return;
    }

    // Swap back
    students[fromIndex].examRoom = lastSwap.roomFrom;
    students[toIndex].examRoom = lastSwap.roomTo;
    undoAvailable = 0;

    FILE *log = fopen("C:\\Users\\visma\\Desktop\\C Programming\\JOB Project\\session_log.txt", "a");
    if (log) {
        fprintf(log, "Undo: Restored %d to Room %d, %d to Room %d\n",
                lastSwap.rollFrom, lastSwap.roomFrom,
                lastSwap.rollTo, lastSwap.roomTo);
        fclose(log);
    }

    printf("Undo successful. Room assignments restored.\n");
}

// Save updated data to CSV
void saveUpdatedData() {
    FILE *fp = fopen("C:\\Users\\visma\\Desktop\\C Programming\\JOB Project\\updated_allotment.csv", "w");
    if (!fp) {
        printf("Failed to write updated data.\n");
        return;
    }

    fprintf(fp, "RollNo,Name,Room\n");
    for (int i = 0; i < numStudents; i++) {
        fprintf(fp, "%d,%s,%d\n", students[i].rollNo, students[i].name, students[i].examRoom);
    }

    fclose(fp);
    printf("Updated data saved to updated_allotment.csv.\n");
}

// Show all students in a given room (numbered list)
void showRoomOccupancy() {
    int room;
    printf("Enter room number (1-%d): ", MAX_ROOMS);
    scanf("%d", &room);

    if (room < 1 || room > MAX_ROOMS) {
        printf("Invalid room number.\n");
        return;
    }

    printf("Students in Room %d:\n", room);
    int found = 0;
    int count = 1;
    for (int i = 0; i < numStudents; i++) {
        if (students[i].examRoom == room) {
            printf("%d. %s (Roll No: %d)\n", count++, students[i].name, students[i].rollNo);
            found = 1;
        }
    }

    if (!found) {
        printf("No students found in this room.\n");
    }
}

int main() {
    printf("Enter maximum number of students per room (default is 20): ");
    if (scanf("%d", &maxPerRoom) != 1 || maxPerRoom <= 0 || maxPerRoom > 50) {
        printf("Invalid input. Using default value 20.\n");
        maxPerRoom = 20;
        // Clear input buffer
        while(getchar() != '\n');
    }

    readDataAndAssignRooms();

    int choice;
    do {
        printf("\n--- Exam Room Allotment System ---\n");
        printf("1. Check room by roll number\n");
        printf("2. Replace student from one room to another\n");
        printf("3. Show all students in a room\n");
        printf("4. Save updated allotment to CSV\n");
        printf("5. Undo last replacement\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            // Clear invalid input
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: searchByRollNumber(); break;
            case 2: replaceStudentInRoom(); break;
            case 3: showRoomOccupancy(); break;
            case 4: saveUpdatedData(); break;
            case 5: undoLastReplacement(); break;
            case 6: printf("Exiting. Thank you.\n"); break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 6);

    return 0;
}


