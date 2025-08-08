
/* main function run properly
*  
* more clean ui - 
*/ 


#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
using namespace std;



//Structure declaration

struct SeminarSlot{
	string date;
	int timeIndex; // Index for timeslots[]
	int duration;
	char hall;

};

struct User {
	string userID;
};

// Function Declaration
bool isSlotAvailable(string date, int timeIndex, char hall, vector<SeminarSlot>& schedule);
string promptValidDate(const string& message);
int promptValidTime(string time[],int timeCount);
void saveToFile(const vector<SeminarSlot>& schedule);
void loadFromFile(vector<SeminarSlot> schedule);
void appendToFile(const SeminarSlot& s);
void displaySeminarDetails(string date, int timeIndex, int duration, char hall, string timeSlot[]);
void printSchedule(const vector<SeminarSlot>& schedule, string timeSlot[]);
void displayTimeVenue(string timeSlot[], char halls[], int timeCount, int hallCount);
void clearScreen();


int main() {
	// Available time slots and halls
	string timeSlot[] = { "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00", "17:00", "18:00" };
	char halls[] = { 'A', 'B', 'C', 'D' };
	int timeCount = 10;
	int hallCount = 4;

	vector<SeminarSlot> schedule;
	

	string date;
	char hall = ' ';// Prevent null value
	int duration = -1;
	string inputTime;
	int timeIndex = -1;


	for (int i = 0; i < 4; i++) {
		displaySeminarDetails(date, timeIndex, duration, hall, timeSlot);

		switch (i) {
		// Prompt date
		case 0:
			date = promptValidDate("Enter a date for your seminar (DD-MM-YYYY): ");
		
		// Prompt time
		case 1:
			timeIndex = promptValidTime(timeSlot, timeCount);

		// Prompt duration
		case 2:
			do {
				cout << "Enter the duration for your seminar in hours (1-10) ";
				cin >> duration;
			} while (duration < 0 || duration > 10);

		// Prompt hall
		case 3:
			do {
				cout << "Enter seminar hall of your choice (A-D): ";
				cin >> hall;
				hall = toupper(hall);
			} while (hall != 'A' && hall != 'B' && hall != 'C' && hall != 'D');
		}

	}
	

	if (isSlotAvailable(date, timeIndex, hall, schedule) == true) {
		SeminarSlot seminar = { date, timeIndex, duration, hall };
		schedule.push_back(seminar);
	}

	printSchedule(schedule, timeSlot);

	return 0;
}

// Function Definition
bool isSlotAvailable(string date, int timeIndex, char hall, vector<SeminarSlot>& schedule) {
	for (auto& slot : schedule) {
		if (slot.date == date && slot.timeIndex == timeIndex && slot.hall == hall) {
			return false;
		}
	}
	// If not found, assume it’s available
	return true;
}

string promptValidDate(const string& message) {
	string date;
	// Check format
	regex datePattern("(\\d{2})-(\\d{2})-(\\d{4})");

	int day, month, year;
	char dash;

	while (true) {
		cout << message;
		cin >> date;

		// 1. Format check
		if (!regex_match(date, datePattern)) {
			cout << "Invalid format! Use DD-MM-YYYY. \n";
			cout << "Please try again...\n\n";
			continue;
		}

		// 2. Parse date
		stringstream ss(date);
		ss >> day >> dash >> month >> dash >> year;

		// 3. Check available month
		if (month < 1 || month > 12) {
			cout << "There is no month " << month;
			cout << ".\nPlease try again...\n\n";
			continue;
		}

		// 4. Check available day based on month
		if (day > 0) {
			if (month == 2) {
				int maxDay;
				// Checking if it's leap year
				if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
					maxDay = 29;
				else maxDay = 28;

				if (day > maxDay) {
					cout << "February " << year << " has only " << maxDay << " days.\n";
					cout << "Please try again...\n\n";
					continue;
				}
			}
			else if (month == 4 || month == 6 || month == 9 || month == 11) {
				if (day > 30) {
					cout << "Month " << month << " has only 30 days.\n";
					cout << "Please try again...\n\n";
					continue;
				}
			}
			else
				if (day > 31) {
					cout << "Month " << month << " has only 31 days.\n";
					cout << "Please try again...\n\n";
					continue;
				}
		}
		else {
			cout << "Enter a valid date.";
			continue;
		}
		// If all validations check passed
		break;
	} 
	return date;
}

int promptValidTime(string time[], int timeCount) {

	string startTime;
	regex timePattern("^([0][9]|[1][0-8]):00");
	do { 
		cout << "Enter the start time for your seminar (HH:MM): ";
		cin >> startTime;
		
		if (!regex_match(startTime, timePattern)) {
			cout << "Enter a valid time!\n";
			cout << "Please try again...\n\n";
		}

	} while (!regex_match(startTime, timePattern));

	for (int i = 0; i < timeCount; i++) {
		if (startTime == time[i]) {
			return i;
		}
	}
}

// Overwrite file with entire schedule vector record  
void saveToFile(const vector<SeminarSlot>& schedule) {
	ofstream out("seminars.txt");
	for (const auto& s : schedule) {
		out << s.date << "|" << s.timeIndex << "|" << s.duration << "|" << s.hall << endl;
	}
	out.close();
}

void loadFromFile(vector<SeminarSlot> schedule) {
	ifstream in("seminars.txt");
	char delimiter;
	SeminarSlot s;
	while (in >> s.date >> delimiter >> s.timeIndex >> delimiter >> s.duration >> delimiter >> s.hall) {
		schedule.push_back(s);
	}
}

// Append new record to existing records in file
void appendToFile(const SeminarSlot& s) {
	ofstream out("seminars.txt", ios::app); // Append mode
	if (out.is_open()) {
		out << s.date << "|" << s.timeIndex << "|" << s.duration << "|" << s.hall << endl;
		out.close();
	}
	else {
		cout << "Error opening file for writing!" << endl;
	}
}

void displaySeminarDetails(string date, int timeIndex, int duration, char hall, string timeSlot[]) {
	cout << format("|{:<7}: {:<15}|\n", "Date", date);
	cout << format("|{:<7}: {:<15}|\n", "Time", ((timeIndex != -1) ? timeSlot[timeIndex] : ""));
	cout << format("|{:<7}: {:<15}|\n", "Duration", duration);
	cout << format("|{:<7}: {:<15}|\n", "Hall", hall);
	cout << "--------------------------------------\n";
}

void printSchedule(const vector<SeminarSlot>& schedule, string timeSlot[]) {
	for (auto& slot : schedule) {
		cout << format("\n|{:<7}: {:<15}|\n", "Date", slot.date);
		cout << format("|{:<7}: {:<15}|\n", "Time", ((slot.timeIndex != -1) ? timeSlot[slot.timeIndex] : ""));
		cout << format("|{:<7}: {:<15}|\n", "Duration", slot.duration);
		cout << format("|{:<7}: {:<15}|\n", "Hall", slot.hall);
	}
}

void displayTimeVenue(string timeSlot[], char halls[], int timeCount, int hallCount) {
	cout << "Available Time Slots: ";
	for (int i = 0; i < timeCount; i++) {
		cout << "[" << time << "]" << " ";
	}

	cout << "\nAvailable Halls: ";
	for (int i = 0; i < hallCount; i++) {
		cout << halls[i] << " ";
	}
	cout << endl << endl;

}

void clearScreen() {
	system("cls");
}