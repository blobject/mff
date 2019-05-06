// file: core.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// global variables

let DEBUG = true;

// math
const TAU = Math.PI * 2;
const SIN = Math.sin;
const COS = Math.cos;
const FLR = Math.floor;
const SGN = Math.sign;
const RAN = Math.random;

// world
let W =
{
  // dimensions
  cv: null, // html canvas object
  c: null,  // canvas context object
  gap: 40,  // space for bar
  w: null,  // width
  h: null,  // height
  w2: null, // half width
  h2: null, // half height

  // colors
  x: [], // particle hue cache
  t: [   // themes
    [0, 0, 240, 290, 0, 40],
  ],

  // animation
  paused: true, // whether currently paused
  fps: 30,      // frames per second
};

// state, load/save-ready
let S =
{
  // a priori, defaults
  n: 2000,  // number of particles
  z: 5,     // particle radius
  d: 8.0,   // density
  a: 180,   // alpha
  b: 17,    // beta
  g: 0.134, // gamma

  // a posteriori
  s: null,  // particle speed
  r: null,  // neighborhood radius
  r2: null, // radius squared
  x: null,  // crowdedness / color interval
};

// particles
let P;

////////////////////////////////////////////////////////////////////////
// core computation

let E =
{

  // the particle class
  Pt: function (x, y, f)
  {
    // uniform distribution
    this.x = x ? x : RAN() * W.w;
    this.y = y ? y : RAN() * W.h;

    // main formula:
    // delta(phi) / delta(t) = a + b * N(t,r) * sgn(R(t,r) - L(t,r))
    this.f = f ? f : RAN() * TAU;
    this.n = 0;
    this.l = 0;
    this.r = 0;
    this.s = SIN(this.f);
    this.c = COS(this.f);
  },

  init: function (keep)
  {
    if (!keep)
    {
      P = new Array(S.n);
      for (let i = 0; i < S.n; i++)
      {
        P[i] = new E.Pt();
      }
    }

    // a posteriori parameter derivation
    S.r = Math.sqrt(W.w * W.h * S.d / S.n / Math.PI);
    S.r2 = S.r * S.r;
    S.s = S.g * S.r;
  },

  tick: function ()
  {
    B.paint();
    //E.next();
    E.next_naive();
  },

  // motion, range-search + kd-tree implementation
  next: function ()
  {
    // localised for performance
    let p = P;
    let s = S;
    let w = W.w;
    let h = W.h;
    let w2 = W.w2;
    let h2 = W.h2;
    let tau = TAU;
    let sin = SIN;
    let cos = COS;
    let flr = FLR;
    let sgn = SGN;
    let mod = U.mod;

    // WIP
  },

  // motion, naive implementation
  next_naive: function ()
  {
    // localised for performance
    let p = P;
    let s = S;
    let w = W.w;
    let h = W.h;
    let w2 = W.w2;
    let h2 = W.h2;
    let tau = TAU;
    let sin = SIN;
    let cos = COS;
    let flr = FLR;
    let sgn = SGN;
    let mod = U.mod;
    let pi;
    let pj;

    // reset particles (N, L, R, sin(F), cos(F))
    for (let i = 0; i < s.n; i++)
    {
      pi = p[i];
      pi.n = 0;
      pi.l = 0;
      pi.r = 0;
      pi.s = sin(pi.f);
      pi.c = cos(pi.f);
    }

    // setup grids then count neighbors
    // https://github.com/nagualdesign/Primordial-Particle-System
    let grid_x_num = s.r > w ? 1 : flr(w / s.r);
    let grid_y_num = s.r > h ? 1 : flr(h / s.r);
    let grid_x_sz = w / grid_x_num;
    let grid_y_sz = h / grid_y_num;

    let grid = new Array(grid_x_num);
    for (let i = 0; i < grid_x_num; i++)
    {
      grid[i] = new Array(grid_y_num);
      for (j = 0; j < grid_y_num; j++)
      {
        grid[i][j] = [];
      }
    }

    for (let i = 0; i < s.n; i++)
    {
      grid[flr(p[i].x / grid_x_sz)][flr(p[i].y / grid_y_sz)].push(i);
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
              for (let q = 0; q < grid[mx][my].length; q++)
              {
                let j = grid[mx][my][q];
                if (i < j)
                {
                  pi = P[i];
                  pj = P[j];
                  let dx = mod(pj.x - pi.x + w2, w) - w2;
                  let dy = mod(pj.y - pi.y + h2, h) - h2;
                  if (s.r2 >= dx * dx + dy * dy)
                  {
                    pi.n++;
                    pj.n++;
                    if (0 > dx * pi.s - dy * pi.c)
                    {
                      pi.r++;
                    }
                    else
                    {
                      pi.l++;
                    }
                    if (0 < dx * pj.s - dy * pj.c)
                    {
                      pj.r++;
                    }
                    else
                    {
                      pj.l++;
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
    for (let i = 0; i < s.n; i++)
    {
      sum += p[i].n;
    }
    s.x = flr(sum / s.n);

    // update particles based on their new neighborhood
    for (let i = 0; i < s.n; i++)
    {
      pi = p[i];
      let delta = s.a + (s.b * pi.n * sgn(pi.r - pi.l));
      pi.f = mod(pi.f + delta, tau);
      pi.x = mod(pi.x + s.s * pi.c, w);
      pi.y = mod(pi.y + s.s * pi.s, h);
    }
  },
};
