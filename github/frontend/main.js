const menuToggle = document.querySelector(".menu-toggle");
const nav = document.querySelector(".nav nav");

if (menuToggle && nav) {
  menuToggle.addEventListener("click", () => {
    const open = nav.style.display === "flex";
    nav.style.display = open ? "none" : "flex";
    nav.style.flexDirection = "column";
    nav.style.position = "absolute";
    nav.style.top = "82px";
    nav.style.left = "0";
    nav.style.right = "0";
    nav.style.padding = "22px";
    nav.style.background = "var(--paper)";
  });
}
