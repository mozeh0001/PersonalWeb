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


// V2 goes correspondingly
document.addEventListener('scroll', () => {
    const hiddenElements = document.querySelectorAll('.hidden');

    hiddenElements.forEach((el) => {
        const elementTop = el.getBoundingClientRect().top;
        const elementHeight = el.offsetHeight;
        const windowHeight = window.innerHeight;

        // Calculate the visibility percentage of the element
        const visibleHeight = Math.min(windowHeight, elementHeight + elementTop) - Math.max(0, elementTop);
        const visibility = Math.max(0, visibleHeight / elementHeight);

        // Adjust classes based on the visibility percentage
        if (visibility > 0.05) {
            el.classList.add('show');
            el.style.transform = `translateX(${(1 - visibility) * -100}%)`;
        } else {
            el.classList.remove('show');
            el.style.transform = `translateX(-100%)`;
        }
    });
});
