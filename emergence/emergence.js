////////////////////////////////////////////////////////////////////////
// globals

// canvas
let canvas = document.querySelector("canvas");
canvas.width = window.innerWidth;
canvas.height = window.innerHeight - 42;
let w = canvas.width;
let h = canvas.height;
let c = canvas.getContext("2d");

// constants
const pi = Math.PI;
const tau = 2 * pi;
const rand = Math.random;

// world: time
let fps = 3;
let now;
let then = Date.now();
let interval = 1000 / fps;
let delta;

// world: particle
let n = 100

function point(x, y, r, c) {
    c.fillRect(x, y, r, r);
}

function animate() {
    requestAnimationFrame(animate);
    now = Date.now();
    delta = now - then;
    if (delta > interval) {
        then = now - (delta % interval);
        // draw code
        c.clearRect(0, 0, w, h);
        for (var i = 0; i < n; i++) {
            point(rand() * w, rand() * h, 3, c);
        }
    }
}

animate();
