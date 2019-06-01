// file: emergence/core.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// this file can be used headlessly

function headless()
{
  return !(typeof window != "undefined" && window.document);
}

const U = (headless() ? require("./util") : UTIL);

////////////////////////////////////////////////////////////////////////
// global variables

const DEBUG = false;

// Color themes.
const COLORS =
[ //  bar       canvas     labels
  [0, "#76c4ae", "#bee9e4", "#000000"], // green
  [1, "#363631", "#16161d", "#ffffff"], // black
  [2, "#ebebeb", "#ffffff", "#000000"], // white
  [3, "#fed6f1", "#fff2e9", "#000000"], // pink
  [4, "#cae6fc", "#daedfd", "#000000"], // blue
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
  w: "width",
  h: "height",
  t: "stop",
  o: "distribution",
  f: "fps",
  n: "num",
  z: "psize",
  r: "radius",
  d: "density",
  a: "alpha",
  b: "beta",
  g: "gamma",
};

// World state, part 1.
// - Global state variables that do not get saved/loaded/historicised.
// - `theme` is special in that the index is user-controllable but not
//   themes themselves.
const WORLD =
{
  // dimension
  canvas:      null, // html canvas object
  context:     null, // canvas context object
  width_half:  null, // half width
  height_half: null, // half height
  gap:         61,   // space for bar

  // coloring
  hues:  [],        // particle hue cache
  theme: COLORS[0], // theme: gr, cy, bl, vi, re, or

  // animation
  paused:       true, // whether currently paused
  tick:         0,    // tick count
  history_size: 100,  // history buffer merge interval

  // miscellaneous
  radius_squared: null,
  speed: null, // particle speed
  crowd: null, // crowdedness / color interval
};

// Default values for `STATE`.
const STATED =
{
  width:        null,  // width
  height:       null,  // height
  stop:         0,     // ticks until stop
  distribution: 0,     // initial particle distribution scheme
  fps:          30,    // frames per second
  num:          4000,  // number of particles
  psize:        3,     // particle size
  radius:       null,  // neighborhood radius
  density:      8.0,   // neighborhood density
  alpha:        180,
  beta:         17,
  gamma:        0.134, // radius-dependent speed scaling
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
  snaps: new Array(WORLD.history_size), // snapshots, sync. with #slider
  buf: new Array(WORLD.history_size),   // buffer
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

    // Initial position
    if (x >= 0)
    {
      this.x = x;
    }
    else
    {
      if      (s.distribution === 0) { f = u.uniform; }
      else if (s.distribution === 1) { f = u.gaussian; }
      else if (s.distribution === 2) { f = u.middle; }
      this.x = f(s.width);
    }

    if (y >= 0)
    {
      this.y = y;
    }
    else
    {
      if      (s.distribution === 0) { f = u.uniform; }
      else if (s.distribution === 1) { f = u.gaussian; }
      else if (s.distribution === 2) { f = u.middle; }
      this.y = f(s.height);
    }

    // Main formula (see original article):
    // delta(phi) per delta(t) = a + b * N(t,r) * sgn(R(t,r) - L(t,r))
    this.phi = (!!phi ? phi : Math.random() * u.TAU);
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

    s.width = (!s.width ? U.auto_width() : s.width);
    s.height = (!s.height ? U.auto_height(w.gap) : s.height);

    if (fresh)
    {
      PTS = new Array(s.num);
      for (let i = 0; i < s.num; i++)
      {
        PTS[i] = new CORE.Pt();
      }
    }

    if (!s.radius)
    {
      s.radius = U.auto_radius(s.width, s.height, s.density, s.num);
      w.radius_squared = s.radius * s.radius;
      w.speed = s.gamma * s.radius;
    }
  },

  // What to do on every frame of animation.
  // - Binds to HTML: `BRDG.toggle` -> `VIEW.toggle` -> `setInterval`.
  tick: function ()
  {
    const c = CORE;
    const b = BRDG;
    const w = WORLD;
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
    const u = U;
    const s = STATE;
    const p = PTS;
    const tau = u.TAU;
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
      CORE.grid_init(state.width, state.height,
                     state.radius, pts, floor),
      mod);
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
      delta = state.alpha + (state.beta * p.n * sgn(p.r - p.l));
      p.phi = mod(p.phi + delta, tau);
      p.sin = sin(p.phi);
      p.cos = cos(p.phi);
      p.x = mod(p.x + world.speed * p.cos, state.width);
      p.y = mod(p.y + world.speed * p.sin, state.height);
    }
  },

  // Helper for `near`.
  // - Initialise a grid whose units have size `radius`.
  grid_init: function (width, height, radius, pts, floor)
  {
    // Number of rows & columns
    const cols = (radius > width ? 1 : floor(width / radius));
    const rows = (radius > height ? 1 : floor(height / radius));
    // Size of horizontal & vertical grid unit
    const xs = width / cols;
    const ys = height / rows;

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

    const w2 = world.width_half;
    const h2 = world.height_half;
    const a = pts[ai];
    const b = pts[bi];
    const dx = mod(b.x - a.x + w2, state.width) - w2;
    const dy = mod(b.y - a.y + h2, state.height) - h2;

    // If the fixed radius exceeds the distance
    if (world.radius_squared >= dx * dx + dy * dy)
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

  // Given a (complete) state object in abbreviated form, set the global
  // state.
  set: function (o)
  {
    const a = ABBREV;
    const s = STATE;

    U.for_abbrev(a, function (key)
    {
      s[a[key]] = o[key];
    });

    if (o.r) { WORLD.radius_squared = o.r * o.r; }
    WORLD.width_half = s.width / 2;
    WORLD.height_half = s.height / 2;

    PTS = o.p;
  },

  // Hand over particles.
  get_pts: function ()
  {
    return PTS;
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
      w: STATE.width,
      h: STATE.height,
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
      tmp[i] = lines[i].replace(/[ \t]+/g, " ").split(" ");
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

    // Remove illegal entries completely for overlapping input (ie.
    // file input + argument input) to work correctly
    for (let key in o)
    {
      if (U.is_nil(o[key]))
      {
        delete o[key];
      }
    }

    for (let i = 1; i < lines.length; i++)
    {
      if (lines[i].replace(/\s+/g, "") != "")
      {
        o.p[i - 1] =
        {
          x: parseFloat(tmp[i][1]),
          y: parseFloat(tmp[i][2]),
          phi: parseFloat(tmp[i][3]),
        }
      }
    }

    // Again, remove p if empty
    if (o.p.length === 0)
    {
      delete o.p;
    }

    return o;
  },

  // Given a state object in abbreviated form, return its string
  // encoding.
  encode: function (width, height, o)
  {
    // Ignore width and height when encode is called, ie. when not
    // headless.
    let s = width.toString() + " " +
            height.toString() + " " +
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
  sanitise: function (o, fresh)
  {
    const a = ABBREV;
    const w = WORLD;
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
          (["t", "f", "n", "z", "d", "g"].includes(key) &&
           value < 0))
      {
        if (key === "w")
        {
          if (fresh)
          {
            params[key] = U.auto_width();
          }
          else
          {
            params[key] = s.width;
          }
        }
        else if (key === "h")
        {
          if (fresh)
          {
            params[key] = U.auto_height(w.gap);
          }
          else
          {
            params[key] = s.height;
          }
        }
        else
        {
          // Set value in STATED
          params[key] = ds[a[key]];
        }
      }
    });

    // Treat radius separately because it requires other params
    if (U.is_nil(params.r) || params.r <= 0)
    {
      params.r = U.auto_radius(params.w, params.h, params.d, params.n);
    }
    w.radius_squared = params.r * params.r;
    w.speed = params.g * params.r;

    // Treat particles
    const np = new Array(params.n); // num has precedence wrt. size
    let nl = 0;

    if (!!o.p && o.p.length > 0)
    {
      let p;
      for (let i = 0; i < (o.p.length > params.n ? params.n : o.p.length);
           i++)
      {
        p = o.p[i];
        np[i] = new CORE.Pt(p.x, p.y, p.phi);
        np[i].n = 0;
        np[i].l = 0;
        np[i].r = 0;
        np[i].sin = Math.sin(p.phi);
        np[i].cos = Math.cos(p.phi);
      }
      nl = o.p.length;
    }
    else if (!!PTS && PTS.length > 0)
    {
      for (let i = 0; i < params.n; i++)
      {
        np[i] = PTS[i];
      }
      nl = PTS.length;
    }
    for (let i = nl; i < params.n; i++)
    {
      np[i] = new CORE.Pt();
    }
    params.p = np;

    return params;
  },
};

////////////////////////////////////////////////////////////////////////
// headless

if (headless())
{
  require("./headless").exec(CORE, WORLD, STATE);
}

