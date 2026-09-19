document.addEventListener("DOMContentLoaded", function () {
    const API_BASE = "http://172.17.223.95:5000";

    const params = new URLSearchParams(window.location.search);
    const trustsealId = params.get("id") || "TS-1048";

    const idEl = document.getElementById("trustsealId");
    const heroIdEl = document.getElementById("heroId");
    const customerEl = document.getElementById("customerName");
    const orderEl = document.getElementById("orderId");
    const packageEl = document.getElementById("packageName");
    const sealedEl = document.getElementById("sealedAt");
    const restaurantEl = document.getElementById("restaurantName");
    const deviceEl = document.getElementById("deviceId");
    const statusEl = document.getElementById("statusText");
    const statusDetailEl = document.getElementById("statusDetail");
    const timelineEl = document.getElementById("timeline");
    const verifyButton = document.getElementById("verifyButton");
    const messageEl = document.getElementById("verificationMessage");

    idEl.textContent = trustsealId;
    heroIdEl.textContent = "TRUSTSEAL · " + trustsealId;

    function renderTimeline(events) {
        timelineEl.innerHTML = "";

        (events || []).forEach(function (event, index) {
            const row = document.createElement("div");
            row.className = "timeline-event";

            const dotClass = index === 0
                ? "timeline-dot active"
                : index === events.length - 1
                    ? "timeline-dot final"
                    : "timeline-dot";

            row.innerHTML = `
                <div class="time">${event.time || "—"}</div>
                <div class="${dotClass}"></div>
                <div class="event-card">
                    <span>${event.title || "EVENT"}</span>
                    <h3>${event.detail || "TrustSeal event recorded"}</h3>
                    <p>${event.title || ""}</p>
                    <small>✓ EVENT RECORDED</small>
                </div>
            `;

            timelineEl.appendChild(row);
        });
    }

    async function loadPackage() {
        try {
            const response = await fetch(
                `${API_BASE}/api/package/${encodeURIComponent(trustsealId)}`
            );

            const data = await response.json();

            if (!response.ok) {
                throw new Error(data.error || "Package not found");
            }

            customerEl.textContent = data.customer?.name || "—";
            orderEl.textContent = data.order_id || "—";
            packageEl.textContent = data.package || "—";
            sealedEl.textContent = data.sealed_at || "—";
            restaurantEl.textContent = data.restaurant?.name || "—";
            deviceEl.textContent = data.device_id || "—";

            statusEl.textContent = data.status || "UNKNOWN";
            statusDetailEl.textContent =
                data.tampered
                    ? "Tamper state reported by the monitoring system."
                    : "TrustSeal record loaded successfully.";

            renderTimeline(data.events);
        } catch (error) {
            statusEl.textContent = "RECORD UNAVAILABLE";
            statusDetailEl.textContent = error.message;
            timelineEl.innerHTML = `
                <div class="timeline-event">
                    <div class="time">ERROR</div>
                    <div class="timeline-dot final"></div>
                    <div class="event-card">
                        <span>TRUSTSEAL</span>
                        <h3>Unable to load package</h3>
                        <p>${error.message}</p>
                        <small>CHECK BACKEND CONNECTION</small>
                    </div>
                </div>
            `;
        }
    }

    verifyButton.addEventListener("click", async function () {
        verifyButton.disabled = true;
        verifyButton.textContent = "CHECKING PACKAGE...";
        messageEl.textContent = "Reading package history...";

        setTimeout(() => {
            messageEl.textContent = "Checking sensor events...";
        }, 800);

        setTimeout(() => {
            messageEl.textContent = "Verifying event record...";
        }, 1600);

        try {
            const response = await fetch(`${API_BASE}/api/verify`, {
                method: "POST",
                headers: {"Content-Type": "application/json"},
                body: JSON.stringify({trustseal_id: trustsealId})
            });

            const data = await response.json();

            if (!response.ok) throw new Error(data.error || "Verification failed");

            setTimeout(() => {
                verifyButton.textContent = "DELIVERY VERIFIED ✓";
                verifyButton.style.background = "#159B99";
                messageEl.textContent =
                    "Prototype confirmation recorded. Production should use OTP/authentication.";
            }, 2400);
        } catch (error) {
            verifyButton.disabled = false;
            verifyButton.textContent = "CONFIRM RECEIPT";
            messageEl.textContent = "Verification failed: " + error.message;
        }
    });

    // Same mobile navigation behavior as the reference site.
    const menuButton = document.getElementById("menuButton");
    const mobileNav = document.getElementById("mobileNav");

    if (menuButton && mobileNav) {
        menuButton.addEventListener("click", function () {
            mobileNav.classList.toggle("open");
            menuButton.textContent =
                mobileNav.classList.contains("open") ? "×" : "☰";
        });

        mobileNav.querySelectorAll("a").forEach(function (link) {
            link.addEventListener("click", function () {
                mobileNav.classList.remove("open");
                menuButton.textContent = "☰";
            });
        });
    }

    loadPackage();
});
