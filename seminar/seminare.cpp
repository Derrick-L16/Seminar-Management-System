#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
using namespace std;

//Structure declaration

struct SeminarSlot {
	string date = "";
	int timeIndex = -1; // Index for timeslots[]
	int duration = 0;
	char hall = ' ';
	int bookingID = 0; // Added booking ID for payment tracking
};

struct User {
	string userID;
};

// Payment Module Structures - Marcus
struct BookingDetail {
	int bookingID = 0;
	string date = "";
	string timeSlot = "";
	int duration = 0;
	char hall = ' ';
	bool isPaid = false;
};

struct PaymentRecord {
	int bookingID = 0;
	string seminarName = "";
	double amountPaid = 0.0;
	string paymentDate = "";
	string paymentStatus = ""; // "Paid" or "Cancelled"
};

struct PaymentMethod {
	int id;
	string name;
	string details;
};

// Function Declaration - Original
bool isSlotAvailable(string date, int timeIndex, char hall, vector<SeminarSlot>& schedule);
bool hasTimeConflict(string date, int timeIndex, int duration, char hall, vector<SeminarSlot>& schedule);
string promptValidDate(const string& message);
int promptValidTime(string time[], int timeCount);
void saveToFile(const vector<SeminarSlot>& schedule);
void loadFromFile(vector<SeminarSlot>& schedule);
void appendToFile(const SeminarSlot& s);
void displaySeminarDetails(string date, int timeIndex, int duration, char hall, string timeSlot[]);
void printSchedule(const vector<SeminarSlot>& schedule, string timeSlot[]);
void displayTimeVenue(string timeSlot[], char halls[], int timeCount, int hallCount);
void clearScreen();

// Payment Module Function Declaration - Marcus
void loadBookingData(vector<BookingDetail>& bookings, string timeSlot[]);
double calculateAmount(int duration);
void makePayment(vector<BookingDetail>& bookings, vector<PaymentRecord>& payments);
void displayPaymentMethods(vector<PaymentMethod>& methods);
int selectPaymentMethod(vector<PaymentMethod>& methods);
void savePaymentRecord(const PaymentRecord& payment);
void displayReceiptWithMethod(const PaymentRecord& payment, const BookingDetail& booking, const string& paymentMethod);
void paymentMenu(vector<SeminarSlot>& schedule, string timeSlot[]);
string getCurrentDate();
int getNextBookingID();
void saveBookingID(int bookingID);

int main() {
	// Available time slots and halls
	string timeSlot[] = { "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00", "17:00", "18:00" };
	char halls[] = { 'A', 'B', 'C', 'D' };
	int timeCount = 10;
	int hallCount = 4;

	vector<SeminarSlot> schedule;
	loadFromFile(schedule);

	int choice;
	do {
		cout << "\n========== SEMINAR BOOKING SYSTEM ==========\n";
		cout << "1. Book a Seminar\n";
		cout << "2. View Schedule\n";
		cout << "3. Payment Module\n";
		cout << "4. Exit\n";
		cout << "==========================================\n";
		cout << "Enter your choice: ";
		cin >> choice;

		switch (choice) {
		case 1: {
			string date;
			char hall = ' ';
			int duration = -1;
			int timeIndex = -1;
			bool exitBooking = false;

			// Date input with exit option
			cout << "\n--- SEMINAR BOOKING ---\n";
			cout << "Enter '999' at any step to return to main menu\n\n";

			while (true) {
				cout << "Enter a date for your seminar (DD-MM-YYYY) or 999 to exit: ";
				string input;
				cin >> input;

				if (input == "999") {
					exitBooking = true;
					break;
				}

				// Validate date format
				regex datePattern("(\\d{2})-(\\d{2})-(\\d{4})");
				if (regex_match(input, datePattern)) {
					stringstream ss(input);
					int day, month, year;
					char dash;
					ss >> day >> dash >> month >> dash >> year;

					// Basic date validation
					if (month >= 1 && month <= 12 && day >= 1 && day <= 31) {
						date = input;
						break;
					}
				}
				cout << "Invalid date format! Please use DD-MM-YYYY format.\n";
			}

			if (exitBooking) break;

			// Time input with exit option
			while (true) {
				cout << "\nAvailable time slots: ";
				for (int i = 0; i < timeCount; i++) {
					cout << "[" << i + 1 << "] " << timeSlot[i] << " ";
				}
				cout << "\nEnter time slot number (1-10) or 999 to exit: ";

				int timeChoice;
				cin >> timeChoice;

				if (timeChoice == 999) {
					exitBooking = true;
					break;
				}

				if (timeChoice >= 1 && timeChoice <= timeCount) {
					timeIndex = timeChoice - 1;
					break;
				}
				cout << "Invalid choice! Please select 1-10.\n";
			}

			if (exitBooking) break;

			// Duration input with exit option
			while (true) {
				cout << "Enter the duration for your seminar in hours (1-10) or 999 to exit: ";
				cin >> duration;

				if (duration == 999) {
					exitBooking = true;
					break;
				}

				if (duration >= 1 && duration <= 10) {
					break;
				}
				cout << "Duration must be between 1 and 10 hours!\n";
			}

			if (exitBooking) break;

			// Hall input with exit option
			while (true) {
				cout << "Enter seminar hall of your choice (A-D) or type 999 to exit: ";
				string hallInput;
				cin >> hallInput;

				if (hallInput == "999") {
					exitBooking = true;
					break;
				}

				if (hallInput.length() == 1) {
					hall = toupper(hallInput[0]);
					if (hall >= 'A' && hall <= 'D') {
						break;
					}
				}
				cout << "Please enter a valid hall (A-D)!\n";
			}

			if (exitBooking) break;

			// Check for time conflicts and handle rebooking
			bool bookingSuccessful = false;
			do {
				if (hasTimeConflict(date, timeIndex, duration, hall, schedule)) {
					cout << "\n[ERROR] Time conflict detected!\n";
					cout << "Sorry, Hall " << hall << " is already booked during your requested time.\n";
					cout << "The selected time slot conflicts with existing bookings.\n";
					cout << "Booking from " << timeSlot[timeIndex] << " for " << duration << " hours overlaps with existing seminars.\n";

					// Show conflicting bookings
					cout << "\nConflicting bookings on " << date << " in Hall " << hall << ":\n";
					for (const auto& slot : schedule) {
						if (slot.date == date && slot.hall == hall) {
							int endTime = slot.timeIndex + slot.duration;
							int requestedEndTime = timeIndex + duration;

							// Check if there's overlap
							if ((timeIndex >= slot.timeIndex && timeIndex < endTime) ||
								(requestedEndTime > slot.timeIndex && timeIndex < slot.timeIndex) ||
								(timeIndex <= slot.timeIndex && requestedEndTime > endTime)) {

								cout << "- " << timeSlot[slot.timeIndex] << " to ";
								if (endTime < timeCount) {
									cout << timeSlot[endTime];
								}
								else {
									cout << timeSlot[timeCount - 1] << "+";
								}
								cout << " (Booking ID: " << slot.bookingID << ")\n";
							}
						}
					}

					cout << "\nPlease choose a different time or different hall.\n";

					// Give user options
					int conflictChoice;
					do {
						cout << "\nWhat would you like to do?\n";
						cout << "1. Choose a different time slot\n";
						cout << "2. Choose a different hall\n";
						cout << "3. Exit booking\n";
						cout << "Enter your choice (1-3): ";
						cin >> conflictChoice;

						if (conflictChoice < 1 || conflictChoice > 3) {
							cout << "Please enter a valid choice (1-3).\n";
						}
					} while (conflictChoice < 1 || conflictChoice > 3);

					if (conflictChoice == 1) {
						// Choose different time slot
						while (true) {
							cout << "\nAvailable time slots: ";
							for (int i = 0; i < timeCount; i++) {
								cout << "[" << i + 1 << "] " << timeSlot[i] << " ";
							}
							cout << "\nEnter new time slot number (1-10) or 999 to exit: ";

							int timeChoice;
							cin >> timeChoice;

							if (timeChoice == 999) {
								exitBooking = true;
								break;
							}

							if (timeChoice >= 1 && timeChoice <= timeCount) {
								timeIndex = timeChoice - 1;
								break;
							}
							cout << "Invalid choice! Please select 1-10.\n";
						}

						if (exitBooking) break;
					}
					else if (conflictChoice == 2) {
						// Choose different hall
						while (true) {
							cout << "Enter different seminar hall (A-D) or type 999 to exit: ";
							string hallInput;
							cin >> hallInput;

							if (hallInput == "999") {
								exitBooking = true;
								break;
							}

							if (hallInput.length() == 1) {
								char newHall = toupper(hallInput[0]);
								if (newHall >= 'A' && newHall <= 'D') {
									hall = newHall;
									break;
								}
							}
							cout << "Please enter a valid hall (A-D)!\n";
						}

						if (exitBooking) break;
					}
					else { // choice == 3
						exitBooking = true;
						break;
					}
				}
				else if (isSlotAvailable(date, timeIndex, hall, schedule)) {
					int bookingID = getNextBookingID();
					SeminarSlot seminar = { date, timeIndex, duration, hall, bookingID };
					schedule.push_back(seminar);
					saveToFile(schedule);
					saveBookingID(bookingID);
					cout << "\n[SUCCESS] Seminar booked successfully! Booking ID: " << bookingID << "\n";
					displaySeminarDetails(date, timeIndex, duration, hall, timeSlot);
					bookingSuccessful = true;
				}
				else {
					cout << "\n[ERROR] Sorry, this exact slot is not available!\n";
					bookingSuccessful = true; // Exit the loop even if booking failed
				}
			} while (!bookingSuccessful && !exitBooking);
			break;
		}
		case 2:
			cout << "\n========== CURRENT SCHEDULE ==========\n";
			if (schedule.empty()) {
				cout << "No seminars scheduled.\n";
			}
			else {
				printSchedule(schedule, timeSlot);
			}
			break;
		case 3:
			paymentMenu(schedule, timeSlot);
			break;
		case 4:
			cout << "Thank you for using the Seminar Booking System!\n";
			break;
		default:
			cout << "Invalid choice! Please try again.\n";
		}
	} while (choice != 4);

	return 0;
}

// Original Function Definitions
bool isSlotAvailable(string date, int timeIndex, char hall, vector<SeminarSlot>& schedule) {
	for (auto& slot : schedule) {
		if (slot.date == date && slot.timeIndex == timeIndex && slot.hall == hall) {
			return false;
		}
	}
	return true;
}

// New function to check for time conflicts with duration
bool hasTimeConflict(string date, int timeIndex, int duration, char hall, vector<SeminarSlot>& schedule) {
	int requestedEndTime = timeIndex + duration;

	for (const auto& slot : schedule) {
		if (slot.date == date && slot.hall == hall) {
			int existingStartTime = slot.timeIndex;
			int existingEndTime = slot.timeIndex + slot.duration;

			// Check for overlap:
			// 1. New booking starts during existing booking
			// 2. New booking ends during existing booking
			// 3. New booking completely contains existing booking
			// 4. Existing booking completely contains new booking
			if ((timeIndex >= existingStartTime && timeIndex < existingEndTime) ||
				(requestedEndTime > existingStartTime && requestedEndTime <= existingEndTime) ||
				(timeIndex <= existingStartTime && requestedEndTime >= existingEndTime) ||
				(existingStartTime >= timeIndex && existingEndTime <= requestedEndTime)) {
				return true; // Conflict detected
			}
		}
	}
	return false; // No conflict
}

string promptValidDate(const string& message) {
	string date;
	regex datePattern("(\\d{2})-(\\d{2})-(\\d{4})");
	int day, month, year;
	char dash;

	while (true) {
		cout << message;
		cin >> date;

		if (!regex_match(date, datePattern)) {
			cout << "Invalid format! Use DD-MM-YYYY. \n";
			cout << "Please try again...\n\n";
			continue;
		}

		stringstream ss(date);
		ss >> day >> dash >> month >> dash >> year;

		if (month < 1 || month > 12) {
			cout << "There is no month " << month;
			cout << ".\nPlease try again...\n\n";
			continue;
		}

		if (day > 0) {
			if (month == 2) {
				int maxDay;
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
	return -1;
}

void saveToFile(const vector<SeminarSlot>& schedule) {
	ofstream out("seminars.txt");
	for (const auto& s : schedule) {
		out << s.date << "|" << s.timeIndex << "|" << s.duration << "|" << s.hall << "|" << s.bookingID << endl;
	}
	out.close();
}

void loadFromFile(vector<SeminarSlot>& schedule) {
	ifstream in("seminars.txt");
	string line;
	while (getline(in, line)) {
		if (line.empty()) continue;

		stringstream ss(line);
		string token;
		SeminarSlot s;
		int field = 0;

		while (getline(ss, token, '|')) {
			switch (field) {
			case 0: s.date = token; break;
			case 1: s.timeIndex = stoi(token); break;
			case 2: s.duration = stoi(token); break;
			case 3: s.hall = token[0]; break;
			case 4: s.bookingID = (token.empty() ? 0 : stoi(token)); break;
			}
			field++;
		}

		if (field >= 4) { // At least the original 4 fields
			if (s.bookingID == 0) {
				s.bookingID = getNextBookingID();
			}
			schedule.push_back(s);
		}
	}
	in.close();
}

void appendToFile(const SeminarSlot& s) {
	ofstream out("seminars.txt", ios::app);
	if (out.is_open()) {
		out << s.date << "|" << s.timeIndex << "|" << s.duration << "|" << s.hall << "|" << s.bookingID << endl;
		out.close();
	}
	else {
		cout << "Error opening file for writing!" << endl;
	}
}

void displaySeminarDetails(string date, int timeIndex, int duration, char hall, string timeSlot[]) {
	cout << format("|{:<7}: {:<15}|\n", "Date", date);
	cout << format("|{:<7}: {:<15}|\n", "Time", ((timeIndex != -1) ? timeSlot[timeIndex] : ""));
	cout << format("|{:<7}: {:<15}|\n", "Duration", to_string(duration) + " hours");
	cout << format("|{:<7}: {:<15}|\n", "Hall", string(1, hall));

	// Show the time range
	if (timeIndex != -1 && timeIndex + duration <= 10) {
		string endTime = timeSlot[timeIndex + duration - 1];
		// Calculate end time (add 1 hour to the last slot)
		int hour = stoi(endTime.substr(0, 2)) + 1;
		string calculatedEndTime = (hour < 10 ? "0" : "") + to_string(hour) + ":00";
		cout << format("|{:<7}: {:<15}|\n", "Range", timeSlot[timeIndex] + " - " + calculatedEndTime);
	}

	cout << "--------------------------------------\n";
}

void printSchedule(const vector<SeminarSlot>& schedule, string timeSlot[]) {
	for (const auto& slot : schedule) {
		cout << format("\n|{:<10}: {:<15}|\n", "Booking ID", slot.bookingID);
		cout << format("|{:<10}: {:<15}|\n", "Date", slot.date);
		cout << format("|{:<10}: {:<15}|\n", "Time", ((slot.timeIndex != -1) ? timeSlot[slot.timeIndex] : ""));
		cout << format("|{:<10}: {:<15}|\n", "Duration", to_string(slot.duration) + " hours");
		cout << format("|{:<10}: {:<15}|\n", "Hall", string(1, slot.hall));

		// Show time range
		if (slot.timeIndex != -1 && slot.timeIndex + slot.duration <= 10) {
			string endTime = timeSlot[slot.timeIndex + slot.duration - 1];
			int hour = stoi(endTime.substr(0, 2)) + 1;
			string calculatedEndTime = (hour < 10 ? "0" : "") + to_string(hour) + ":00";
			cout << format("|{:<10}: {:<15}|\n", "Range", timeSlot[slot.timeIndex] + " - " + calculatedEndTime);
		}

		cout << "--------------------------------------\n";
	}
}

void displayTimeVenue(string timeSlot[], char halls[], int timeCount, int hallCount) {
	cout << "Available Time Slots: ";
	for (int i = 0; i < timeCount; i++) {
		cout << "[" << timeSlot[i] << "]" << " ";
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

// Payment Module Function Definitions - Marcus

void loadBookingData(vector<BookingDetail>& bookings, string timeSlot[]) {
	bookings.clear();
	ifstream in("seminars.txt");
	ifstream paidFile("payments.txt");

	// Load paid booking IDs
	vector<int> paidBookingIDs;
	string line;
	while (getline(paidFile, line)) {
		if (line.empty()) continue;
		stringstream ss(line);
		string token;
		int field = 0;
		int bookingID = 0;
		string status;

		while (getline(ss, token, '|')) {
			if (field == 0) bookingID = stoi(token);
			if (field == 4) status = token;
			field++;
		}

		if (status == "Paid") {
			paidBookingIDs.push_back(bookingID);
		}
	}
	paidFile.close();

	// Load bookings
	while (getline(in, line)) {
		if (line.empty()) continue;

		stringstream ss(line);
		string token;
		BookingDetail booking;
		int field = 0;

		while (getline(ss, token, '|')) {
			switch (field) {
			case 0: booking.date = token; break;
			case 1: {
				int timeIndex = stoi(token);
				booking.timeSlot = (timeIndex >= 0 && timeIndex < 10) ? timeSlot[timeIndex] : "Unknown";
				break;
			}
			case 2: booking.duration = stoi(token); break;
			case 3: booking.hall = token[0]; break;
			case 4: booking.bookingID = (token.empty() ? 0 : stoi(token)); break;
			}
			field++;
		}

		if (field >= 4) {
			if (booking.bookingID == 0) {
				booking.bookingID = getNextBookingID();
			}

			// Check if already paid
			booking.isPaid = false;
			for (int paidID : paidBookingIDs) {
				if (paidID == booking.bookingID) {
					booking.isPaid = true;
					break;
				}
			}

			bookings.push_back(booking);
		}
	}
	in.close();
}

double calculateAmount(int duration) {
	const double HOURLY_RATE = 30.0; // RM30 per hour
	if (duration <= 0) return 0.0;
	return duration * HOURLY_RATE;
}

void makePayment(vector<BookingDetail>& bookings, vector<PaymentRecord>& payments) {
	if (bookings.empty()) {
		cout << "\n[ERROR] No bookings available for payment.\n";
		return;
	}

	// Display available bookings
	cout << "\n============================= AVAILABLE BOOKINGS FOR PAYMENT =============================\n";
	bool hasUnpaidBookings = false;

	for (const auto& booking : bookings) {
		if (!booking.isPaid) {
			hasUnpaidBookings = true;
			cout << format("ID: {:<3} | Date: {:<10} | Time: {:<5} | Duration: {:<1}h | Hall: {} | Amount: RM{:.2f}\n",
				booking.bookingID, booking.date, booking.timeSlot,
				booking.duration, booking.hall, calculateAmount(booking.duration));
		}
	}

	if (!hasUnpaidBookings) {
		cout << "All bookings have been paid.\n";
		return;
	}

	cout << "==========================================================================================\n";

	int bookingID;
	bool validBookingID = false;
	BookingDetail selectedBooking;

	// Booking ID validation loop with exit option
	do {
		cout << "Enter Booking ID to make payment (or 999 to return to payment menu): ";
		cin >> bookingID;

		// Check for exit option
		if (bookingID == 999) {
			cout << "Returning to payment menu...\n";
			return;
		}

		// Check if booking exists and is not paid
		for (const auto& booking : bookings) {
			if (booking.bookingID == bookingID) {
				if (booking.isPaid) {
					cout << "[ERROR] This booking has already been paid!\n";
					break;
				}
				else {
					validBookingID = true;
					selectedBooking = booking;
					break;
				}
			}
		}

		if (!validBookingID) {
			cout << "[ERROR] Invalid Booking ID or booking already paid. Please try again.\n";
		}

	} while (!validBookingID);

	// Display booking details and amount
	cout << "\n========== PAYMENT DETAILS ==========\n";
	cout << format("Booking ID: {}\n", selectedBooking.bookingID);
	cout << format("Date: {}\n", selectedBooking.date);
	cout << format("Time: {}\n", selectedBooking.timeSlot);
	cout << format("Duration: {} hours\n", selectedBooking.duration);
	cout << format("Hall: {}\n", selectedBooking.hall);

	double amount = calculateAmount(selectedBooking.duration);
	cout << format("Amount Due: RM{:.2f}\n", amount);
	cout << "====================================\n";

	// Initialize payment methods
	vector<PaymentMethod> paymentMethods = {
		{1, "TNG e-Wallet", "Touch 'n Go Digital Payment"},
		{2, "GrabPay", "Grab e-Wallet Payment"},
		{3, "Boost", "Boost e-Wallet Payment"},
		{4, "Maybank QR Pay", "Maybank QR Code Payment"},
		{5, "CIMB Pay", "CIMB Bank e-Payment"},
		{6, "Public Bank e-Pay", "Public Bank Online Payment"},
		{7, "Credit Card", "Visa/MasterCard Payment"},
		{8, "Online Banking", "Direct Bank Transfer"}
	};

	// Payment method selection
	int selectedMethodId = selectPaymentMethod(paymentMethods);
	if (selectedMethodId == -1) {
		cout << "Payment cancelled. Returning to payment menu...\n";
		return;
	}

	string selectedMethodName = "";
	for (const auto& method : paymentMethods) {
		if (method.id == selectedMethodId) {
			selectedMethodName = method.name;
			break;
		}
	}

	// Payment confirmation loop
	char confirm;
	do {
		cout << "\n========== PAYMENT CONFIRMATION ==========\n";
		cout << format("Payment Method: {}\n", selectedMethodName);
		cout << format("Amount: RM{:.2f}\n", amount);
		cout << "==========================================\n";
		cout << "Confirm payment? (Y/N): ";
		cin >> confirm;
		confirm = toupper(confirm);

		if (confirm != 'Y' && confirm != 'N') {
			cout << "Please enter Y for Yes or N for No.\n";
		}
	} while (confirm != 'Y' && confirm != 'N');

	// Process payment
	PaymentRecord payment;
	payment.bookingID = selectedBooking.bookingID;
	payment.seminarName = "Seminar " + selectedBooking.date + " " + selectedBooking.timeSlot + " via " + selectedMethodName;
	payment.amountPaid = amount;
	payment.paymentDate = getCurrentDate();

	if (confirm == 'Y') {
		payment.paymentStatus = "Paid";
		cout << "\n[SUCCESS] Payment processed successfully via " << selectedMethodName << "!\n";

		// Save payment record
		savePaymentRecord(payment);

		// Mark booking as paid
		for (auto& booking : bookings) {
			if (booking.bookingID == selectedBooking.bookingID) {
				booking.isPaid = true;
				break;
			}
		}

		// Display receipt with payment method
		displayReceiptWithMethod(payment, selectedBooking, selectedMethodName);

	}
	else {
		payment.paymentStatus = "Cancelled";
		savePaymentRecord(payment);
		cout << "\n[CANCELLED] Payment cancelled.\n";
	}

	payments.push_back(payment);
}

void displayPaymentMethods(vector<PaymentMethod>& methods) {
	cout << "\n========== AVAILABLE PAYMENT METHODS ==========\n";
	for (const auto& method : methods) {
		cout << format("[{}] {} - {}\n", method.id, method.name, method.details);
	}
	cout << "[0] Cancel Payment\n";
	cout << "===============================================\n";
}

int selectPaymentMethod(vector<PaymentMethod>& methods) {
	int choice;
	bool validChoice = false;

	do {
		displayPaymentMethods(methods);
		cout << "Select payment method (1-8) or 0 to cancel: ";
		cin >> choice;

		if (choice == 0) {
			return -1; // Cancel payment
		}

		// Validate choice
		for (const auto& method : methods) {
			if (method.id == choice) {
				validChoice = true;
				break;
			}
		}

		if (!validChoice) {
			cout << "[ERROR] Invalid choice! Please select a valid payment method.\n";
		}

	} while (!validChoice);

	return choice;
}

// Enhanced receipt display function (add this new function)
void displayReceiptWithMethod(const PaymentRecord& payment, const BookingDetail& booking, const string& paymentMethod) {
	cout << "\n========== PAYMENT RECEIPT ==========\n";
	cout << format("Receipt Date: {}\n", payment.paymentDate);
	cout << format("Booking ID: {}\n", payment.bookingID);
	cout << format("Seminar Date: {}\n", booking.date);
	cout << format("Time Slot: {}\n", booking.timeSlot);
	cout << format("Duration: {} hours\n", booking.duration);
	cout << format("Hall: {}\n", booking.hall);
	cout << format("Payment Method: {}\n", paymentMethod);
	cout << format("Amount Paid: RM{:.2f}\n", payment.amountPaid);
	cout << format("Status: {}\n", payment.paymentStatus);
	cout << "====================================\n";
	cout << "Thank you for your payment!\n";
}

void savePaymentRecord(const PaymentRecord& payment) {
	ofstream out("payments.txt", ios::app);
	if (out.is_open()) {
		out << payment.bookingID << "|" << payment.seminarName << "|"
			<< fixed << setprecision(2) << payment.amountPaid << "|"
			<< payment.paymentDate << "|" << payment.paymentStatus << endl;
		out.close();
	}
	else {
		cout << "Error saving payment record!" << endl;
	}
}

void displayReceipt(const PaymentRecord& payment, const BookingDetail& booking) {
	cout << "\n========== PAYMENT RECEIPT ==========\n";
	cout << format("Receipt Date: {}\n", payment.paymentDate);
	cout << format("Booking ID: {}\n", payment.bookingID);
	cout << format("Seminar Date: {}\n", booking.date);
	cout << format("Time Slot: {}\n", booking.timeSlot);
	cout << format("Duration: {} hours\n", booking.duration);
	cout << format("Hall: {}\n", booking.hall);
	cout << format("Amount Paid: RM{:.2f}\n", payment.amountPaid);
	cout << format("Status: {}\n", payment.paymentStatus);
	cout << "====================================\n";
	cout << "Thank you for your payment!\n";
}

void paymentMenu(vector<SeminarSlot>& schedule, string timeSlot[]) {
	vector<BookingDetail> bookings;
	vector<PaymentRecord> payments;

	int choice;
	do {
		cout << "\n========== PAYMENT MODULE ==========\n";
		cout << "1. Make Payment\n";
		cout << "2. View Payment History\n";
		cout << "3. Back to Main Menu\n";
		cout << "===================================\n";
		cout << "Enter your choice: ";
		cin >> choice;

		switch (choice) {
		case 1:
			loadBookingData(bookings, timeSlot);
			makePayment(bookings, payments);
			break;
		case 2: {
			ifstream in("payments.txt");
			string line;
			bool hasPayments = false;

			cout << "\n================================= PAYMENT HISTORY =================================\n";
			while (getline(in, line)) {
				if (line.empty()) continue;
				hasPayments = true;

				stringstream ss(line);
				string token;
				int field = 0;
				vector<string> fields;

				while (getline(ss, token, '|')) {
					fields.push_back(token);
				}

				if (fields.size() >= 5) {
					cout << format("Booking ID: {} | Amount: RM{} | Date: {} | Status: {}\n",
						fields[0], fields[2], fields[3], fields[4]);
				}
			}

			if (!hasPayments) {
				cout << "No payment records found.\n";
			}
			cout << "===================================================================================\n";
			in.close();
			break;
		}
		case 3:
			cout << "Returning to main menu...\n";
			break;
		default:
			cout << "Invalid choice! Please try again.\n";
		}
	} while (choice != 3);
}

string getCurrentDate() {
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
	string str(buffer);

	return str;
}

int getNextBookingID() {
	ifstream in("booking_counter.txt");
	int counter = 1000; // Start from 1000

	if (in.is_open()) {
		in >> counter;
		in.close();
	}

	counter++;

	ofstream out("booking_counter.txt");
	if (out.is_open()) {
		out << counter;
		out.close();
	}

	return counter;
}

void saveBookingID(int bookingID) {
	ofstream out("booking_counter.txt");
	if (out.is_open()) {
		out << bookingID;
		out.close();
	}
}