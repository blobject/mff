// file: emergence/util.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// this file is used headlessly

function headless()
{
  return !(typeof window != "undefined" && window.document);
}

////////////////////////////////////////////////////////////////////////
// utility

const UTIL =
{

  // numeric constants /////////////////////////////////////////////////

  TAU: Math.PI * 2,

  NDISTR: 3, // number of distribution options

  // javascript ////////////////////////////////////////////////////////

  is_empty: function (o)
  {
    return Object.keys(o).length === 0 && o.constructor === Object;
  },

  is_nil: function (v)
  {
    return (Number.isNaN(v) || v === null || v === undefined);
  },

  clone: function (o)
  {
    return JSON.parse(JSON.stringify(o));
  },

  // math //////////////////////////////////////////////////////////////

  mod: function (n, b)
  {
    n %= b;
    if (n < 0)
    {
      n += b;
    }
    return n;
  },

  rad: function (n)
  {
    return n / 360 * UTIL.TAU;
  },

  deg: function (n)
  {
    return n * 360 / UTIL.TAU;
  },

  uniform: function (n)
  {
    return Math.random() * n;
  },

  gaussian: function (n)
  {
    let a = 0;
    let b = 0;
    while (a === 0) { a = Math.random(); }
    while (b === 0) { b = Math.random(); }
    let x = Math.sqrt(-2 * Math.log(a)) * Math.cos(UTIL.TAU * b);
    x = (x / 10) + 0.5;
    if (x > 1 || x < 0) { return UTIL.gaussian(n); }
    return x * n;
  },

  // TODO: this can be reconsidered and improved
  middle: function (n)
  {
    const factor = 4;
    return (Math.random() * n / factor) + (n / 2) - (n / 2 / factor);
  },

  // Generate random alpha, beta, and gamma values
  randomise: function ()
  {
    return ({
      a: (Math.random() * 360 - 180).toFixed(2),
      b: (Math.random() * 180 - 90).toFixed(2),
      g: (Math.random() * 0.5).toFixed(2),
    });
  },

  // parameters ////////////////////////////////////////////////////////

  // Prepare state for core computation.
  // - Currently, this includes only converting the alpha and beta
  //   parameters into radians.
  prep: function (state)
  {
    const u = UTIL;
    const o = u.clone(state);
    if (o.alpha) { o.alpha = u.rad(o.alpha); }
    if (o.beta) { o.beta = u.rad(o.beta); }
    return o;
  },

  auto_width: function ()
  {
    if (headless())
    {
      return 1000;
    }
    else
    {
      return window.innerWidth;
    }
  },

  auto_height: function (gap)
  {
    if (headless())
    {
      return 1000;
    }
    else
    {
      return window.innerHeight - gap;
    }
  },

  // A predefined neighborhood radius value, if not provided by user.
  // - Taken from https://github.com/nagualdesign/Primordial-Particle-System
  auto_radius: function (width, height, density, num)
  {
    const r = Math.round(Math.sqrt(width * height * density /
                                   num / Math.PI));
    return r > 0 ? r : 1;
  },

  // Helper for iterating through all parameter abbreviations.
  for_abbrev: function (params, f)
  {
    for (let key in params)
    {
      if (params.hasOwnProperty(key))
      {
        f(key);
      }
    }
  },

  // Print out some of the world state.
  debug_params: function (world, state)
  {
    const o = state.distribution;
    console.log("EMERGENCE\n---------" +
                "\nwidth   = " + state.width +
                "\nheight  = " + state.height +
                "\nstop    = " + state.stop +
                "\ndistr.  = " + (o === 0 ? "uniform" : (o === 1 ? "gaussian" : (o === 2 ? "middle" : "ERROR"))) +
                "\nfps     = " + state.fps +
                "\nnum     = " + state.num +
                "\npt-size = " + state.psize +
                "\nradius  = " + state.radius +
                "\ndensity = " + state.density +
                "\nalpha   = " + state.alpha +
                "\nbeta    = " + state.beta +
                "\ngamma   = " + state.gamma +
                "\nspeed   = " + world.speed +
                "\n---------");
  },
};

if (headless())
{
  module.exports =
  {
    TAU: UTIL.TAU,
    NDISTR: UTIL.NDISTR,
    is_nil: UTIL.is_nil,
    clone: UTIL.clone,
    mod: UTIL.mod,
    uniform: UTIL.uniform,
    gaussian: UTIL.gaussian,
    middle: UTIL.middle,
    prep: UTIL.prep,
    auto_width: UTIL.auto_width,
    auto_height: UTIL.auto_height,
    auto_radius: UTIL.auto_radius,
    for_abbrev: UTIL.for_abbrev,
  };
}

