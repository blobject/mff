// by  : jooh@cuni.cz
// for : nprg045
// note: work in progress

////////////////////////////////////////////////////////////////////////
// global variables

// program
let DEBUG = 1; // 0 = false; 1 = true

// canvas
let canvas;
let C;
let W;
let H;
let W2;
let H2;

// constants
const PI = Math.PI;
const TAU = 2 * PI;
const RAND = Math.random;
const SIN = Math.sin;
const COS = Math.cos;
const FLOOR = Math.floor;
const SQRT = Math.sqrt;
const SGN = Math.sign;

// time, animation, drawing
let paused = true;
let FPS = 30;
let colors = new Array();

// matter, particles, parameters
let pts;      // list of particles
let N = 2000; // number of particles
let Z = 5;    // particle radius
let D = 8.0;  // density
let ALPHA = 180;
let BETA = 17;
let GAMMA = 0.134;
let V;       // particle velocity
let R;       // neighborhood radius
let R2;      // radius squared
let X;       // crowdedness / color interval

////////////////////////////////////////////////////////////////////////
// objects

function Pt()
{
  // uniform distribution
  this.x = RAND() * W;
  this.y = RAND() * H;

  // motion law (formula 1, Schmickl et al.):
  // delta(phi)/delta(t) = alpha+beta*N(t,r)*sign(R(t,r)-L(t,r))
  this.n = 0;
  this.l = 0;
  this.r = 0;
  this.f = RAND() * TAU;
  this.s = SIN(this.f);
  this.c = COS(this.f);
}

////////////////////////////////////////////////////////////////////////
// functions: core

// motion implementation (figure 1, Schmickl et al.):
function next()
{
  // reset particles (N, L, R, sin(F), cos(F))
  for (let i = 0; i < N; i++)
  {
    pts[i].n = 0;
    pts[i].l = 0;
    pts[i].r = 0;
    pts[i].s = SIN(pts[i].f);
    pts[i].c = COS(pts[i].f);
  }

  // core computation: setup grids then count neighbors
  // (blatantly copied from https://github.com/nagualdesign/Primordial-Particle-System)
  let grid_x_num = R > W ? 1 : FLOOR(W / R);
  let grid_y_num = R > H ? 1 : FLOOR(H / R);
  let grid_x_sz = W / grid_x_num;
  let grid_y_sz = H / grid_y_num;

  let grid = new Array(grid_x_num);
  for (let i = 0; i < grid_x_num; i++)
  {
    grid[i] = new Array(grid_y_num);
    for (j = 0; j < grid_y_num; j++)
    {
      grid[i][j] = new Array();
    }
  }

  for (let i = 0; i < N; i++)
  {
    grid[FLOOR(pts[i].x / grid_x_sz)][FLOOR(pts[i].y / grid_y_sz)].push(i);
  }

  for (let gx = 0; gx < grid_x_num; gx++)
  {
    for (let gy = 0; gy < grid_y_num; gy++)
    {
      for (let gp = 0; gp < grid[gx][gy].length; gp++)
      {
        let i = grid[gx][gy][gp];
        for (cx = gx - 1; cx <= gx + 1; cx++)
        {
          for (cy = gy - 1; cy <= gy + 1; cy++)
          {
            let mx = mod(cx, grid_x_num);
            let my = mod(cy, grid_y_num);
            for (let p = 0; p < grid[mx][my].length; p++)
            {
              let j = grid[mx][my][p];
              if (i < j)
              {
                let dx = mod(pts[j].x - pts[i].x + W2, W) - W2;
                let dy = mod(pts[j].y - pts[i].y + H2, H) - H2;
                if (R2 >= dx * dx + dy * dy)
                {
                  pts[i].n++;
                  pts[j].n++;
                  if (0 > dx * pts[i].s - dy * pts[i].c)
                  {
                    pts[i].r++;
                  }
                  else
                  {
                    pts[i].l++;
                  }
                  if (0 < dx * pts[j].s - dy * pts[j].c)
                  {
                    pts[j].r++;
                  }
                  else
                  {
                    pts[j].l++;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // update crowdedness based on new neighborhood average
  let sum = 0;
  for (let i = 0; i < N; i++)
  {
    sum += pts[i].n;
  }
  X = FLOOR(sum / N);

  // update particles based on their new neighborhood
  for (let i = 0; i < N; i++)
  {
    let delta = ALPHA + (BETA * pts[i].n * SGN(pts[i].r - pts[i].l));
    pts[i].f = mod(pts[i].f + delta, TAU);
    pts[i].x = mod(pts[i].x + V * pts[i].c, W);
    pts[i].y = mod(pts[i].y + V * pts[i].s, H);
  }
}

////////////////////////////////////////////////////////////////////////
// functions: world manipulation

function tick()
{
  draw();
  next();
}

function draw()
{
  C.clearRect(0, 0, W, H);

  for (let i = 0; i < N; i++)
  {
    C.fillStyle = "hsl(" + color(pts[i].n) + ",100%,50%)"
    C.fillRect(pts[i].x, pts[i].y, Z, Z);
  }
}

function world_reset()
{
  if (!paused)
  {
    canvas_toggle();
  }
  canvas_resize();
  parse_params();
  world_init_particles();
  canvas_toggle();
}

function world_init_particles()
{
  // particles
  pts = new Array(N);
  for (let i = 0; i < N; i++)
  {
    pts[i] = new Pt();
  }

  // parameters
  ALPHA = (ALPHA / 360) * TAU; // to radians
  BETA = (BETA / 360) * TAU; // to radians
  R = SQRT(W * H * D / N / PI);
  R2 = R * R;
  V = GAMMA * R;

  if (DEBUG) console.log("EMERGENCE\n---------" +
                         "\nfps   = " + FPS +
                         "\nnum   = " + N +
                         "\ndens  = " + D +
                         "\nalpha = " + ALPHA +
                         "\nbeta  = " + BETA +
                         "\ngamma = " + GAMMA +
                         "\nrad   = " + R +
                         "\nspd   = " + V +
                         "\n---------");
}

function canvas_resize()
{
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight - 48;
  W = canvas.width;
  H = canvas.height;
  W2 = W / 2;
  H2 = H / 2;
}

function canvas_toggle()
{
  if (paused)
  {
    if (DEBUG) console.log("(canvas_toggle) play");
    paused = false;
    animate = setInterval(tick, (1000 / FPS));
  }
  else
  {
    if (DEBUG) console.log("(canvas_toggle) pause");
    paused = true;
    window.clearTimeout(animate);
  }
}

////////////////////////////////////////////////////////////////////////
// functions: preparation & user interface

function world_init()
{
  document.addEventListener("keyup", canvas_key);
  canvas = document.querySelector("canvas");
  C = canvas.getContext("2d");
  world_reset();
}

function canvas_key(event)
{
  switch (event.key)
  {
    case " ":
      canvas_toggle();
      break;

    default:
      console.log("(canvas_key) unrecognised key");
  }
}

////////////////////////////////////////////////////////////////////////
// functions: utility

function mod(n, lim)
{
  n %= lim;
  if (n < 0)
  {
    n += lim;
  }

  return n;
}

function color(n)
{
  if (colors[n] === undefined)
  {
    let color;
    if (n <= X)
    {
      color = 240; // blue
    }
    else if (n <= 2 * X)
    {
      color = 290; // purple
    }
    else if (n <= 5 * X)
    {
      color = 0; // red
    }
    else
    {
      color = 40; // orange
    }
    colors[n] = color;
  }

  return colors[n];
}

function parse_params()
{
  let query = window.location.search.substring(1);
  if (!query)
  {
    return;
  }

  let params = query.split("&");
  if (!check_params(params))
  {
    return;
  }

  for (let i = 0; i < params.length; i++)
  {
    let pair = params[i].split("=");
    let key = decodeURIComponent(pair[0]);
    let val = decodeURIComponent(pair[1]);
    let v;

    switch (key)
    {
      case "n":
        v = parseInt(val);
        if (v < 1 || v > 20000)
        {
          console.log("(parse_params) value of \"" + key +
                      "\" must be 1 <= x <= 20000");
          return;
        }
        N = v;
        break;

      case "f":
        v = parseInt(val);
        if (v < 1 || v > 500)
        {
          console.log("(parse_params) value of \"" + key +
                      "\" must be 1 <= x <= 500");
          return;
        }
        FPS = v;
        break;

      case "z":
        v = parseInt(val);
        if (v < 1 || v > 25)
        {
          console.log("(parse_params) value of \"" + key +
                      "\" must be 1 <= x <= 25");
          return;
        }
        Z = v;
        break;

      case "d":
        v = parseFloat(val);
        if (v <= 0.0 || v > 100.0)
        {
          console.log("(parse_params) value of \"" + key +
                      "\" must be 0.0 < x <= 100.0");
        }
        D = v;
        break;

      case "a":
        v = parseFloat(val);
        if (v < -180.0 || v > 180.0)
        {
          console.log("(parse_params) value of \"" + key +
                      "\" must be -180.0 <= x <= 180.0");
        }
        ALPHA = v;
        break;

      case "b":
        v = parseFloat(val);
        if (v < -100.0 || v > 100.0)
        {
          console.log("(parse_params) value of \"" + key +
                      "\" must be -100.0 <= x <= 100.0");
        }
        BETA = v;
        break;

      case "g":
        v = parseFloat(val);
        if (v <= 0.0 || v >= 1.0)
        {
          console.log("(parse_params) value of \"" + key +
                      "\" must be 0.0 < x < 1.0");
        }
        GAMMA = v;
        break;

      default:
        // check_params()
    }
  }
}

function check_params(params)
{
  for (let i = 0; i < params.length; i++)
  {
    let pair = params[i].split("=");
    let k = decodeURIComponent(pair[0]);
    if (!(k === "n" || k === "f" || k === "z" || k === "d" ||
          k === "a" || k === "b" || k === "g"))
    {
      console.log("(check_params) unrecognised parameter \"" + k +
                  "\" (known: [nfzdabg])");
      return false;
    }
    if (isNaN(decodeURIComponent(pair[1])))
    {
      console.log("(check_params) value of \"" + pair[0] +
                  "\" must be numeric");
      return false;
    }
  }

  return true;
}

