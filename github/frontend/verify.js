// Local development:
// const API_BASE = "http://127.0.0.1:5000";
//
// Production:
// replace the value with your deployed Flask backend URL.

const API_BASE = "http://127.0.0.1:5000";

const params = new URLSearchParams(window.location.search);
const trustsealId = params.get("id") || "TS-1048";

const trustsealIdEl = document.getElementById("trustsealId");
const statusBadgeEl = document.getElementById("statusBadge");
const customerNameEl = document.getElementById("customerName");
const orderIdEl = document.getElementById("orderId");
const packageNameEl = document.getElementById("packageName");
const sealedAtEl = document.getElementById("sealedAt");
const restaurantNameEl = document.getElementById("restaurantName");
const restaurantBranchEl = document.getElementById("restaurantBranch");
const restaurantAddressEl = document.getElementById("restaurantAddress");
const timelineEl = document.getElementById("timeline");
const verifyButton = document.getElementById("verifyButton");
const resultEl = document.getElementById("result");

trustsealIdEl.textContent = trustsealId;

function renderStatus(status, tampered = false) {
  statusBadgeEl.textContent = status || "UNKNOWN";
  statusBadgeEl.className = "status-badge";

  if (tampered || String(status).includes("TAMPER")) {
    statusBadgeEl.classList.add("tampered");
  } else if (String(status).includes("CONFIRMED")) {
    statusBadgeEl.classList.add("verified");
  }
}

function renderTimeline(events = []) {
  timelineEl.innerHTML = "";

  events.forEach(event => {
    const item = document.createElement("article");
    item.className = "timeline-item";

    item.innerHTML = `
      <div class="timeline-time">${event.time || "—"}</div>
      <div>
        <h3 class="timeline-title">${event.title || "Event"}</h3>
        <p class="timeline-detail">${event.detail || ""}</p>
      </div>
    `;

    timelineEl.appendChild(item);
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

    customerNameEl.textContent = data.customer?.name || "—";
    orderIdEl.textContent = data.order_id || "—";
    packageNameEl.textContent = data.package || "—";
    sealedAtEl.textContent = data.sealed_at || "—";

    restaurantNameEl.textContent = data.restaurant?.name || "—";
    restaurantBranchEl.textContent = data.restaurant?.branch || "—";
    restaurantAddressEl.textContent = data.restaurant?.address || "—";

    renderStatus(data.status, data.tampered);
    renderTimeline(data.events);
  } catch (error) {
    renderStatus("NOT FOUND", true);
    timelineEl.innerHTML = `
      <div class="timeline-item">
        <div class="timeline-time">ERROR</div>
        <div>
          <h3 class="timeline-title">Unable to load package</h3>
          <p class="timeline-detail">${error.message}</p>
        </div>
      </div>
    `;
  }
}

verifyButton.addEventListener("click", async () => {
  verifyButton.disabled = true;
  verifyButton.textContent = "Confirming...";

  try {
    const response = await fetch(`${API_BASE}/api/verify`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify({
        trustseal_id: trustsealId
      })
    });

    const data = await response.json();

    if (!response.ok) {
      throw new Error(data.error || "Verification failed");
    }

    resultEl.hidden = false;
    resultEl.textContent = "✓ Receipt confirmed. The TrustSeal handover is recorded.";
    renderStatus(data.status);
    verifyButton.textContent = "Receipt confirmed";
  } catch (error) {
    resultEl.hidden = false;
    resultEl.textContent = `Verification failed: ${error.message}`;
    verifyButton.disabled = false;
    verifyButton.textContent = "Confirm receipt";
  }
});

loadPackage();
