// file: emergence/core.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// this file is used headlessly

function headless()
{
  return !(typeof window != "undefined" && window.document);
}

const U = (headless() ? require("./util") : UTIL);

////////////////////////////////////////////////////////////////////////
// global variables

const DEBUG = false;

// Constants.
const TAU = Math.PI * 2;

// Color themes.
const COLORS =
[ // bar       canvas     labels    less --> more crowded
  ["#76c4ae", "#bee9e4", "#000000", 240,290,  0,40], // green
  ["#363631", "#16161d", "#ffffff", 140,320,240,40], // black
  ["#ebebeb", "#ffffff", "#000000", 200,320,240,40], // white
  ["#fed6f1", "#fff2e9", "#000000", 290,240, 20, 0], // pink
  ["#cae6fc", "#daedfd", "#000000", 200,240,  0,40], // blue
];

// Interesting instances of alpha and beta (see original article).
const CONFIGS =
[
  {a: 180, b:  17}, // lifelike structures 1
  {a: 180, b:  -7}, // moving structures
  {a: 180, b: -15}, // clean cow pattern
  {a:  90, b: -21}, // chaos w/ random aggr. 1
  {a:   0, b: -10}, // fingerprint pattern
  {a:   0, b: -41}, // chaos w/ random aggr. 2
  {a:   0, b: -25}, // untidy cow pattern
  {a:-180, b: -48}, // chaos w/ random aggr. 3
  {a:-180, b:   5}, // regular pattern
  {a:-159, b:  15}, // lifelike structures 2
  {a:   0, b:   1}, // stable cluster pattern
  {a:-180, b:  58}, // chaotic pattern 1
  {a:   0, b:  40}, // chaotic pattern 2
  {a:   0, b:   8}, // cells + moving cluster
  {a:   0, b:   0}, // chaotic pattern 3
  {a:  45, b:   4}, // stable rings
];

// State parameter listing, for translation to/from abbreviation.
const ABBREV =
{
  w: "w",
  h: "h",
  t: "stop",
  o: "distr",
  f: "fps",
  n: "num",
  z: "psz",
  r: "rad",
  d: "den",
  a: "a",
  b: "b",
  g: "g",
};

// World state, part 1.
// - Global state variables that do not get saved/loaded/historicised.
// - `theme` is special in that the index is user-controllable but not
//   themes themselves.
const WORLD =
{
  // dimension
  canvas: null, // html canvas object
  c:      null, // canvas context object
  whalf:  null, // half width
  hhalf:  null, // half height
  gap:    61,   // space for bar

  // coloring
  hues:  [],        // particle hue cache
  theme: COLORS[0], // theme: gr, cy, bl, vi, re, or

  // animation
  paused: true, // whether currently paused
  tick:   0,    // tick count
  hbsz:   100,  // history buffer merge interval

  // miscellaneous
  radsq: null, // radius squared
  speed: null, // particle speed
  crowd: null, // crowdedness / color interval
};

// Default values for `STATE`.
const STATED =
{
  w:     null,  // width
  h:     null,  // height
  stop:  0,     // ticks until stop
  distr: 0,     // initial particle distribution scheme
  fps:   30,    // frames per second
  num:   4000,  // number of particles
  psz:   3,     // particle size
  rad:   null,  // neighborhood radius
  den:   8.0,   // density
  a:     180,   // alpha
  b:     17,    // beta
  g:     0.134, // gamma
};

// World state, part 2.
// - Like `WORLD`, but prone to load/save/historicity.
const STATE = U.clone(STATED);

// Particles.
// - A flat list that is also partially prone to load/save/historicity.
let PTS;

// History.
const HIST =
{
  snaps: new Array(WORLD.hbsz), // snapshots, sync. with #slider
  buf: new Array(WORLD.hbsz),   // buffer
};

////////////////////////////////////////////////////////////////////////
// core computation

const CORE =
{

  // The particle class.
  Pt: function (x, y, phi)
  {
    const u = U;
    const s = STATE;
    let f;

    // Initial position control
    if (x)
    {
      this.x = x;
    }
    else
    {
      if      (s.distr === 0) { f = u.uniform; }
      else if (s.distr === 1) { f = u.gaussian; }
      else if (s.distr === 2) { f = u.middle; }
      this.x = f(s.w);
    }

    if (y)
    {
      this.y = y;
    }
    else
    {
      if      (s.distr === 0) { f = u.uniform; }
      else if (s.distr === 1) { f = u.gaussian; }
      else if (s.distr === 2) { f = u.middle; }
      this.y = f(s.h);
    }

    // Main formula (see original article):
    // delta(phi) per delta(t) = a + b * N(t,r) * sgn(R(t,r) - L(t,r))
    this.phi = (phi ? phi : Math.random() * TAU);
    this.n = 0;
    this.l = 0;
    this.r = 0;
    this.sin = Math.sin(this.phi);
    this.cos = Math.cos(this.phi);
  },

  // Initialise particles and immediately derivable parameters.
  init: function (fresh)
  {
    const w = WORLD;
    const s = STATE;

    s.w = (!s.w ? U.auto_width() : s.w);
    s.h = (!s.h ? U.auto_height(w.gap) : s.h);

    if (fresh)
    {
      s.rad = U.auto_radius(s.w, s.h, s.den, s.num);

      PTS = new Array(s.num);
      for (let i = 0; i < s.num; i++)
      {
        PTS[i] = new CORE.Pt();
      }
    }

    w.radsq = s.rad * s.rad;
    w.speed = s.g * s.rad;
  },

  // What to do on every frame of animation.
  // - Binds to HTML: `BRDG.toggle` -> `VIEW.toggle` -> `setInterval`.
  tick: function ()
  {
    const c = CORE;
    const b = BRDG;
    let w = WORLD;
    const s = STATE;

    // Disallow further animation if `stop` is specified
    if (!s.stop || w.tick < s.stop)
    {
      w.tick++;
      b.tally();
      b.paint();
      c.next(c, w);
    }
    else
    {
      b.pause();
      b.put_stop_alert();
    }
  },

  // Motion.
  // - The core of this entire program.
  next: function (core, world)
  {
    // "Localise" globals variables for performance.
    const u = U;
    const s = STATE;
    const p = PTS;
    const tau = TAU;
    const sin = Math.sin;
    const cos = Math.cos;
    const floor = Math.floor;
    const sgn = Math.sign;
    const mod = u.mod;

    core.reset(s, p, sin, cos);
    core.near(world, s, p, floor, mod);
    core.crowd(world, s, p, floor);
    core.move(world, u.prep(s), p, tau, sin, cos, sgn, mod);
  },

  // Reset particle properties.
  // - Namely, N, L, R, sin(F), and cos(F).
  reset: function (state, pts, sin, cos)
  {
    let p;
    for (let i = 0; i < state.num; i++)
    {
      p = pts[i];
      p.n = 0;
      p.l = 0;
      p.r = 0;
      p.sin = sin(p.phi);
      p.cos = cos(p.phi);
    }
  },

  // Calculate N, L, and R for each particle, using naive grid-based
  // distance comparison.
  near: function (world, state, pts, floor, mod)
  {
    CORE.from(world, state, pts,
      CORE.grid_init(state.w, state.h, state.rad, pts, floor), mod);
  },

  // Update crowdedness based on new neighborhood average.
  crowd: function (world, state, pts, floor)
  {
    let sum = 0;
    for (let i = 0; i < state.num; i++)
    {
      sum += pts[i].n;
    }
    world.crowd = floor(sum / state.num);
  },

  // Update particle positions and heading, based on their new
  // neighborhood.
  move: function (world, state, pts, tau, sin, cos, sgn, mod)
  {
    let p;
    for (let i = 0; i < state.num; i++)
    {
      p = pts[i];
      delta = state.a + (state.b * p.n * sgn(p.r - p.l));
      p.phi = mod(p.phi + delta, tau);
      p.sin = sin(p.phi);
      p.cos = cos(p.phi);
      p.x = mod(p.x + world.speed * p.cos, state.w);
      p.y = mod(p.y + world.speed * p.sin, state.h);
    }
  },

  // Helper for `near`.
  // - Initialise a grid for performance optimisation.
  grid_init: function (w, h, rad, pts, floor)
  {
    // Number of rows & columns
    const cols = (rad > w ? 1 : floor(w / rad));
    const rows = (rad > h ? 1 : floor(h / rad));
    // Size of horizontal & vertical grid unit
    const xs = w / cols;
    const ys = h / rows;

    const units = new Array(cols);
    for (let i = 0; i < cols; i++)
    {
      units[i] = new Array(rows);
      for (j = 0; j < rows; j++)
      {
        units[i][j] = [];
      }
    }

    // Fill grid units with indices of resident particles
    for (let i = 0; i < pts.length; i++)
    {
      units[floor(pts[i].x / xs)][floor(pts[i].y / ys)].push(i);
    }

    return ([units, cols, rows]);
  },

  // Helper for `near`.
  // - Recognise the "a" in the a-->b distance calculation.
  from: function (world, state, pts, grid, mod)
  {
    const units = grid[0]; // matrix of grid units holding pt indices
    const cols = grid[1]; // number of horizontal grid units
    const rows = grid[2]; // number of vertical grid units

    // loop through each grid unit
    for (let col = 0; col < cols; col++)
    {
      for (let row = 0; row < rows; row++)
      {
        // loop through each pt index in each unit
        for (let n = 0; n < units[col][row].length; n++)
        {
          CORE.to(world, state, pts, units, cols, rows, col, row,
                  units[col][row][n], mod);
        }
      }
    }
  },

  // Helper for `near`.
  // - Recognise the "b" in the a-->b distance calculation.
  to: function (world, state, pts, units, cols, rows, col, row, a, mod)
  {
    let i;
    let j;

    // Loop through each neighboring unit of the given particle,
    // whose index is "a" and location is grid row "x", col "y"
    for (let c = col - 1; c <= col + 1; c++)
    {
      for (let r = row - 1; r <= row + 1; r++)
      {
        i = mod(c, cols);
        j = mod(r, rows);
        // Loop through each pt index in the unit-neighborhood
        for (let n = 0; n < units[i][j].length; n++)
        {
          CORE.calculate(world, state, pts, a, units[i][j][n], mod);
        }
      }
    }
  },

  // Helper for `near`.
  // - Do the actual distance calculation between a-->b.
  // - Update particle properties (ie. N, L, R) accordingly.
  calculate: function (world, state, pts, ai, bi, mod)
  {
    // avoid redundant calculation between the same two pts.
    if (bi >= ai)
    {
      return;
    }

    const w2 = world.whalf;
    const h2 = world.hhalf;
    const a = pts[ai];
    const b = pts[bi];
    const dx = mod(b.x - a.x + w2, state.w) - w2;
    const dy = mod(b.y - a.y + h2, state.h) - h2;

    // if distance is less than the radius
    if (world.radsq >= dx * dx + dy * dy)
    {
      a.n++;
      b.n++;
      // b to the right of a
      if (0 > dx * a.sin - dy * a.cos)
      {
        a.r++;
      }
      else
      {
        a.l++;
      }
      // a to the right of b
      if (0 < dx * b.sin - dy * b.cos)
      {
        b.r++;
      }
      else
      {
        b.l++;
      }
    }
  },

  //////////////////////////////////////////////////////////////////////
  // The following are included in CORE so that headless execution /////
  // will not have to pull in BRDG. ////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  // Given a (complete) state object in abbreviated form,
  // set the global state.
  set: function (o)
  {
    const a = ABBREV;
    const s = STATE;

    // Set global state.
    U.for_abbrev(a, function (key)
    {
      s[a[key]] = o[key];
    });
    if (o.r) WORLD.radsq = o.r * o.r;
    WORLD.whalf = STATE.w / 2;
    WORLD.hhalf = STATE.h / 2;

    // Set particles
    const np = new Array(s.num);
    let nl;

    if (o.p)
    {
      // STATE.num has precedence over o.p.length
      let p;
      for (let i = 0; i < (o.p.length > s.num ? s.num : o.p.length);
           i++)
      {
        p = o.p[i];
        np[i] = new CORE.Pt(p.x, p.y, p.phi);
        np[i].n = 0;
        np[i].l = 0;
        np[i].r = 0;
        np[i].s = Math.sin(p.phi);
        np[i].c = Math.cos(p.phi);
      }
      nl = o.p.length;
    }

    else
    {
      for (let i = 0; i < s.num; i++)
      {
        np[i] = PTS[i];
      }
      nl = PTS.length;
    }

    // If new STATE.num is greater than either o.p.length (if provided)
    // or the previous PTS.length.
    for (let i = nl; i < s.num; i++)
    {
      np[i] = new CORE.Pt();
    }

    PTS = np;
  },

  // Hand over state in abbreviated form.
  get_params: function ()
  {
    const a = ABBREV;
    const s = STATE;
    const o = {};

    U.for_abbrev(a, function (key)
    {
      if (!["w", "h"].includes(key))
      {
        o[key] = s[a[key]];
      }
    });

    return o;
  },

  // Hand over abbreviated state + more information
  // - This is in turn used by `stringify` for saving/outputting.
  get_snapshot: function ()
  {
    const params =
    {
      w: STATE.w,
      h: STATE.h,
    };

    Object.assign(params, CORE.get_params());

    params.p = [];
    let p;

    for (let i = 0; i < params.n; i++)
    {
      p = PTS[i];
      params.p[i] =
      {
        x: p.x,
        y: p.y,
        phi: p.phi,
      };
    }

    return params;
  },

  // Given a string representing encoded state, return the state as an
  // object in abbreviated form.
  decode: function (s)
  {
    const tmp = [];
    // Vertical whitespace split
    const lines = s.replace(/[\f\n\r\v]+/g, "\n").split("\n");
    for (let i = 0; i < lines.length; i++)
    {
      // Horizontal whitespace split
      tmp[i] = lines[i].replace(/[\s\t]+/g, " ").split(" ");
    }

    const o =
    {
      w: parseInt(tmp[0][0]),
      h: parseInt(tmp[0][1]),
      t: parseInt(tmp[0][2]),
      o: parseInt(tmp[0][3]),
      f: parseInt(tmp[0][4]),
      n: parseInt(tmp[0][5]),
      z: parseInt(tmp[0][6]),
      r: parseFloat(tmp[0][7]),
      d: parseFloat(tmp[0][8]),
      a: parseFloat(tmp[0][9]),
      b: parseFloat(tmp[0][10]),
      g: parseFloat(tmp[0][11]),
      p: [],
    };

    for (let i = 1; i < lines.length; i++)
    {
      if (lines[i].replace(/\s+/g, "") != "")
      {
        o.p[i - 1] =
        {
          x: parseFloat(tmp[i][1]),
          y: parseFloat(tmp[i][2]),
          z: parseFloat(tmp[i][3]),
        }
      }
    }

    return o;
  },

  // Given a state object in abbreviated form, return its string
  // encoding.
  encode: function (w, h, o)
  {
    // ignore width and height when encode is called, ie. when not
    // running headless.
    let s = w.toString() + " " +
            h.toString() + " " +
            (o.t).toString() + " " +
            (o.o).toString() + " " +
            (o.f).toString() + " " +
            (o.n).toString() + " " +
            (o.z).toString() + " " +
            (o.r).toString() + " " +
            (o.d).toString() + " " +
            (o.a).toString() + " " +
            (o.b).toString() + " " +
            (o.g).toString();
    for (let i = 0; i < o.p.length; i++)
    {
      s += "\n" + (i + 1).toString() +
           " " + (o.p[i].x).toString() +
           " " + (o.p[i].y).toString() +
           " " + (o.p[i].phi).toString();
    }
    return s;
  },

  // Check emptiness and validity, falling back to default values.
  sanitise: function (o)
  {
    const a = ABBREV;
    const s = STATE;
    const ds = STATED;
    const params = U.clone(o);
    let value;

    U.for_abbrev(a, function (key)
    {
      value = params[key];

      // If parameter is nil or out of range
      if (U.is_nil(value) ||
          (key === "w" && (value <= 0)) ||
          (key === "h" && (value <= 0)) ||
          (key === "o" && (value < 0 || value > 2)) ||
          (["t", "f", "n", "z", "r", "d", "g"].includes(key) &&
           value < 0))
      {
        if (key === "w")
        {
          params[key] = U.auto_width();
        }
        else if (key === "h")
        {
          params[key] = U.auto_height(WORLD.gap);
        }
        else if (key === "r")
        {
          params[key] = U.auto_radius((params.w ? params.w : s.w),
                                      (params.h ? params.h : s.h),
                                      (params.d ? params.d : s.den),
                                      (params.n ? params.n : s.num));
        }
        else
        {
          params[key] = ds[a[key]];
        }
      }
    });

    return params;
  },
};

////////////////////////////////////////////////////////////////////////
// headless

if (headless())
{
  require("./headless").exec(CORE, WORLD, STATE);
}

