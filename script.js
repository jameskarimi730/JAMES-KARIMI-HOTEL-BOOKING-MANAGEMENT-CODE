
const rooms = [
    { number: 101, type: "Single", price: 1500 },
    { number: 102, type: "Single", price: 1500 },
    { number: 103, type: "Double", price: 2500 },
    { number: 104, type: "Double", price: 2500 },
    { number: 105, type: "Twin", price: 3000 },
    { number: 106, type: "Twin", price: 3000 },
    { number: 107, type: "Deluxe", price: 4500 },
    { number: 108, type: "Deluxe", price: 4500 },
    { number: 109, type: "Executive", price: 6000 },
    { number: 110, type: "Executive", price: 6000 }
];

let bookings = [];

try {
    bookings = JSON.parse(localStorage.getItem("nexoraBookings")) || [];
    if (!Array.isArray(bookings)) bookings = [];
} catch (error) {
    bookings = [];
}

let nextID = Math.max(
    1000,
    ...bookings.map(booking => Number(booking.id) || 1000)
) + 1;

function saveBookings() {
    localStorage.setItem("nexoraBookings", JSON.stringify(bookings));
}

function escapeHTML(value) {
    return String(value).replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    })[character]);
}

function getRoomBooking(roomNumber) {
    return bookings.find(booking =>
        booking.room === roomNumber && booking.active
    );
}

function showSection(sectionID) {
    document.querySelectorAll(".section").forEach(section => {
        section.classList.add("hidden");
    });

    document.getElementById(sectionID).classList.remove("hidden");

    if (sectionID === "dashboard") updateDashboard();
    if (sectionID === "rooms") displayRooms();
    if (sectionID === "bookings") displayBookings();
}

function updateDashboard() {
    const activeBookings = bookings.filter(booking => booking.active);
    const occupiedRooms = new Set(activeBookings.map(b => b.room));
    const totalPayments = activeBookings.reduce(
        (sum, booking) => sum + booking.paid, 0
    );

    document.getElementById("totalRooms").textContent = rooms.length;
    document.getElementById("totalBookings").textContent = activeBookings.length;
    document.getElementById("availableRooms").textContent =
        rooms.length - occupiedRooms.size;

    document.getElementById("totalPayments").textContent =
        "KSh " + totalPayments.toLocaleString("en-KE");
}

function loadRoomOptions() {
    const select = document.getElementById("roomNumber");

    select.innerHTML = '<option value="">Select Available Room</option>';

    rooms.forEach(room => {
        if (!getRoomBooking(room.number)) {
            const option = document.createElement("option");
            option.value = room.number;
            option.textContent =
                `Room ${room.number} - ${room.type} - KSh ${room.price}/night`;
            select.appendChild(option);
        }
    });
}

function displayRooms() {
    const roomList = document.getElementById("roomList");

    roomList.innerHTML = rooms.map(room => {
        const occupied = !!getRoomBooking(room.number);

        return `
            <div class="room-card ${occupied ? "occupied" : ""}">
                <h3>Room ${room.number}</h3>
                <p>Type: ${room.type}</p>
                <p>Price: KSh ${room.price.toLocaleString("en-KE")}</p>
                <p>Status: <strong>${occupied ? "Occupied" : "Available"}</strong></p>
            </div>
        `;
    }).join("");
}

function displayBookings() {
    const bookingList = document.getElementById("bookingList");
    const search = document.getElementById("searchBooking").value
        .trim().toLowerCase();

    const filtered = bookings.filter(booking => {
        return booking.active &&
            (String(booking.id).includes(search) ||
             booking.name.toLowerCase().includes(search));
    });

    if (filtered.length === 0) {
        bookingList.innerHTML = "<p>No matching active bookings found.</p>";
        return;
    }

    bookingList.innerHTML = filtered.map(booking => {
        const balance = booking.total - booking.paid;

        return `
            <div class="booking-card">
                <h3>${escapeHTML(booking.name)}</h3>
                <p>Booking ID: ${booking.id}</p>
                <p>Phone: ${escapeHTML(booking.phone)}</p>
                <p>ID/Passport: ${escapeHTML(booking.idNumber)}</p>
                <p>Room: ${booking.room} (${escapeHTML(booking.type)})</p>
                <p>Nights: ${booking.nights}</p>
                <p>Total: KSh ${booking.total.toLocaleString("en-KE")}</p>
                <p>Paid: KSh ${booking.paid.toLocaleString("en-KE")}</p>
                <p>Balance: KSh ${balance.toLocaleString("en-KE")}</p>
                <button onclick="printReceipt(${booking.id})">
                    Print Receipt
                </button>
                <button class="cancel-btn"
                    onclick="cancelBooking(${booking.id})">
                    Cancel Booking
                </button>
            </div>
        `;
    }).join("");
}

document.getElementById("bookingForm").addEventListener("submit", function(event) {
    event.preventDefault();

    const name = document.getElementById("guestName").value.trim();
    const phone = document.getElementById("phone").value.trim();
    const idNumber = document.getElementById("idNumber").value.trim();
    const roomNumber = Number(document.getElementById("roomNumber").value);
    const nights = Number(document.getElementById("nights").value);

    if (!name || !phone || !idNumber || !roomNumber || nights <= 0 ||
        !Number.isInteger(nights)) {
        alert("Please enter valid guest and booking details.");
        return;
    }

    const room = rooms.find(room => room.number === roomNumber);

    if (!room || getRoomBooking(roomNumber)) {
        alert("This room is not available. Please select another room.");
        loadRoomOptions();
        return;
    }

    const booking = {
        id: nextID++,
        name,
        phone,
        idNumber,
        room: roomNumber,
        type: room.type,
        nights,
        total: room.price * nights,
        paid: 0,
        active: true,
        date: new Date().toISOString()
    };

    bookings.push(booking);
    saveBookings();

    alert(
        "Booking created successfully!\n" +
        "Booking ID: " + booking.id +
        "\nTotal: KSh " + booking.total.toLocaleString("en-KE")
    );

    this.reset();
    loadRoomOptions();
    displayRooms();
    displayBookings();
    updateDashboard();
});

document.getElementById("paymentForm").addEventListener("submit", function(event) {
    event.preventDefault();

    const id = Number(document.getElementById("paymentBookingId").value);
    const amount = Number(document.getElementById("paymentAmount").value);

    const booking = bookings.find(item => item.id === id && item.active);

    if (!booking) {
        alert("Active booking not found.");
        return;
    }

    const balance = booking.total - booking.paid;

    if (!Number.isFinite(amount) || amount <= 0 || amount > balance) {
        alert("Enter an amount greater than zero and not exceeding KSh " + balance);
        return;
    }

    booking.paid += amount;
    saveBookings();

    alert(
        "Payment recorded successfully!\nRemaining balance: KSh " +
        (booking.total - booking.paid).toLocaleString("en-KE")
    );

    this.reset();
    updateDashboard();
    displayBookings();
});

function cancelBooking(id) {
    const booking = bookings.find(item => item.id === id && item.active);

    if (!booking) return;

    if (confirm("Are you sure you want to cancel booking #" + id + "?")) {
        booking.active = false;
        saveBookings();

        alert("Booking cancelled successfully.");

        loadRoomOptions();
        displayRooms();
        displayBookings();
        updateDashboard();
    }
}

function printReceipt(id) {
    const booking = bookings.find(item => item.id === id && item.active);

    if (!booking) return;

    const receiptWindow = window.open("", "_blank");

    if (!receiptWindow) {
        alert("Please allow pop-ups to print the receipt.");
        return;
    }

    const balance = booking.total - booking.paid;

    receiptWindow.document.write(`
        <!DOCTYPE html>
        <html>
        <head>
            <title>Hotel Booking Receipt</title>
            <style>
                body { font-family: Arial; padding: 30px; }
                h2 { text-align: center; }
                hr { margin: 20px 0; }
                button { padding: 10px 20px; }
            </style>
        </head>
        <body>
            <h2>NEXORA GRAND HOTEL</h2>
            <h3 style="text-align:center">PAYMENT RECEIPT</h3>
            <hr>
            <p>Booking ID: ${booking.id}</p>
            <p>Guest: ${escapeHTML(booking.name)}</p>
            <p>Phone: ${escapeHTML(booking.phone)}</p>
            <p>Room: ${booking.room} (${escapeHTML(booking.type)})</p>
            <p>Nights: ${booking.nights}</p>
            <hr>
            <p>Total: KSh ${booking.total.toLocaleString("en-KE")}</p>
            <p>Paid: KSh ${booking.paid.toLocaleString("en-KE")}</p>
            <p>Balance: KSh ${balance.toLocaleString("en-KE")}</p>
            <hr>
            <p>Thank you for choosing Nexora Grand Hotel!</p>
            <button onclick="window.print()">Print Receipt</button>
        </body>
        </html>
    `);

    receiptWindow.document.close();
}

document.getElementById("searchBooking").addEventListener("input", displayBookings);

// Initialize the hotel management system
loadRoomOptions();
displayRooms();
displayBookings();
updateDashboard();
showSection("dashboard");
