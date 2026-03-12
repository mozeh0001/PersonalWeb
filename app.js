// document.addEventListener('scroll', () => {
//     const hiddenElements = document.querySelectorAll('.hidden');

//     hiddenElements.forEach((el) => {
//         const elementTop = el.getBoundingClientRect().top;
//         const windowHeight = window.innerHeight;
//         const threshold = windowHeight * 0.75; // Set the threshold to 80% of the viewport height

//         // Add the show class if the element's top is within the threshold
//         if (elementTop < threshold && elementTop > 0) {
//             el.classList.add('show');
//         } else {
//             el.classList.remove('show');
//         }
//     });
// });


document.addEventListener("scroll", () => {
    const hiddenElements = document.querySelectorAll(".hidden");
    const windowHeight = window.innerHeight;

    hiddenElements.forEach((el) => {
        const rect = el.getBoundingClientRect();
        const elementTop = rect.top;
        const elementBottom = rect.bottom;

        // If any part of the element is visible
        if (elementTop < windowHeight && elementBottom > 0) {
            el.classList.add("show");
        } else {
            el.classList.remove("show");
        }
    });
});
