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

  tau: Math.PI * 2, // not using TAU in core.js because of headless

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
    return n / 360 * UTIL.tau;
  },

  deg: function (n)
  {
    return n * 360 / UTIL.tau;
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
    let x = Math.sqrt(-2 * Math.log(a)) * Math.cos(UTIL.tau * b);
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

  // parameters ////////////////////////////////////////////////////////

  // Prepare state for core computation.
  // - Currently, this includes only converting the alpha and beta
  //   parameters into radians.
  prep: function (state)
  {
    const u = UTIL;
    const o = u.clone(state);
    if (o.a) { o.a = u.rad(o.a); }
    if (o.b) { o.b = u.rad(o.b); }
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
  auto_radius: function (w, h, d, n)
  {
    return Math.round(Math.sqrt(w * h * d / n / Math.PI));
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
    const o = state.distr;
    console.log("EMERGENCE\n---------" +
                "\nwidth  = " + state.w +
                "\nheight = " + state.h +
                "\nstop   = " + state.stop +
                "\ndistr  = " + (o === 0 ? "uniform" : (o === 1 ? "gaussian" : (o === 2 ? "middle" : "ERROR"))) +
                "\nfps    = " + state.fps +
                "\nnum    = " + state.num +
                "\npt-sz  = " + state.psz +
                "\nrad    = " + state.rad +
                "\ndens   = " + state.den +
                "\nalpha  = " + state.a +
                "\nbeta   = " + state.b +
                "\ngamma  = " + state.g +
                "\nspeed  = " + world.speed +
                "\n---------");
  },
};

if (headless())
{
  module.exports =
  {
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

