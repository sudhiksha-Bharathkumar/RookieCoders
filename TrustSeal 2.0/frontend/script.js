document.addEventListener("DOMContentLoaded", function () {
    const menuButton = document.getElementById("menuButton");
    const mobileNav = document.getElementById("mobileNav");
    const navVerify = document.getElementById("navVerify");

    if (menuButton && mobileNav) {
        menuButton.addEventListener("click", function () {
            mobileNav.classList.toggle("open");
            const isOpen = mobileNav.classList.contains("open");
            menuButton.textContent = isOpen ? "×" : "☰";
        });

        mobileNav.querySelectorAll("a").forEach(function (link) {
            link.addEventListener("click", function () {
                mobileNav.classList.remove("open");
                menuButton.textContent = "☰";
            });
        });
    }

    if (navVerify) {
        navVerify.addEventListener("click", function () {
            window.location.href = "verify.html?id=TS-1048";
        });
    }

    document.addEventListener("keydown", function (event) {
        if (event.key === "Escape") {
            if (mobileNav) mobileNav.classList.remove("open");
            if (menuButton) menuButton.textContent = "☰";
        }
    });
});