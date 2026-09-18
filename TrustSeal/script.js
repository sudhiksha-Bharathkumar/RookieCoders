/* =====================================================
   TRUST SEAL
   JAVASCRIPT
===================================================== */


document.addEventListener(
    "DOMContentLoaded",
    function () {


        /* =================================================
           ELEMENTS
        ================================================= */

        const menuButton =
            document.getElementById("menuButton");


        const mobileNav =
            document.getElementById("mobileNav");


        const navVerify =
            document.getElementById("navVerify");


        const viewOrder =
            document.getElementById("viewOrder");


        const verifyButton =
            document.getElementById("verifyButton");


        const verificationMessage =
            document.getElementById(
                "verificationMessage"
            );



        /* =================================================
           SMOOTH SCROLL
        ================================================= */

        function scrollToSection(id) {


            const section =
                document.getElementById(id);


            if (!section) {

                return;

            }


            section.scrollIntoView({

                behavior: "smooth",

                block: "start"

            });

        }



        /* =================================================
           MOBILE MENU
        ================================================= */

        if (
            menuButton &&
            mobileNav
        ) {


            menuButton.addEventListener(
                "click",
                function () {


                    mobileNav.classList.toggle(
                        "open"
                    );


                    const isOpen =
                        mobileNav.classList.contains(
                            "open"
                        );


                    menuButton.textContent =
                        isOpen
                            ? "×"
                            : "☰";


                }
            );


            const mobileLinks =
                mobileNav.querySelectorAll(
                    "a"
                );


            mobileLinks.forEach(
                function (link) {


                    link.addEventListener(
                        "click",
                        function () {


                            mobileNav.classList.remove(
                                "open"
                            );


                            menuButton.textContent =
                                "☰";


                        }
                    );


                }
            );


        }



        /* =================================================
           NAV VERIFY
        ================================================= */

        if (navVerify) {


            navVerify.addEventListener(
                "click",
                function () {

                    scrollToSection(
                        "verify"
                    );

                }
            );


        }



        /* =================================================
           VIEW PACKAGE
        ================================================= */

        if (viewOrder) {


            viewOrder.addEventListener(
                "click",
                function () {

                    scrollToSection(
                        "customer"
                    );

                }
            );


        }



        /* =================================================
           QR VERIFICATION
        ================================================= */

        if (verifyButton) {


            verifyButton.addEventListener(
                "click",
                function () {


                    verifyButton.disabled =
                        true;


                    verifyButton.textContent =
                        "CHECKING PACKAGE...";


                    if (verificationMessage) {

                        verificationMessage.textContent =
                            "Reading package history...";

                    }



                    /* FIRST CHECK */

                    setTimeout(
                        function () {


                            if (
                                verificationMessage
                            ) {

                                verificationMessage.textContent =
                                    "Checking sensor events...";

                            }


                        },
                        800
                    );



                    /* SECOND CHECK */

                    setTimeout(
                        function () {


                            if (
                                verificationMessage
                            ) {

                                verificationMessage.textContent =
                                    "Verifying event chain...";

                            }


                        },
                        1600
                    );



                    /* THIRD CHECK */

                    setTimeout(
                        function () {


                            if (
                                verificationMessage
                            ) {

                                verificationMessage.textContent =
                                    "Confirming authorized handover...";

                            }


                        },
                        2400
                    );



                    /* FINAL RESULT */

                    setTimeout(
                        function () {


                            verifyButton.textContent =
                                "DELIVERY VERIFIED ✓";


                            verifyButton.style.background =
                                "#159B99";


                            if (
                                verificationMessage
                            ) {

                                verificationMessage.textContent =
                                    "Monitoring deactivated. Package verified.";

                            }



                            /* SCROLL TO SUCCESS */

                            setTimeout(
                                function () {


                                    scrollToSection(
                                        "complete"
                                    );


                                },
                                700
                            );


                        },
                        3200
                    );


                }
            );


        }



        /* =================================================
           ESCAPE CLOSES MOBILE MENU
        ================================================= */

        document.addEventListener(
            "keydown",
            function (event) {


                if (
                    event.key === "Escape"
                ) {


                    if (mobileNav) {

                        mobileNav.classList.remove(
                            "open"
                        );

                    }


                    if (menuButton) {

                        menuButton.textContent =
                            "☰";

                    }


                }


            }
        );


    }
);