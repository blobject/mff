// file: util.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// utility

let UTIL =
{

  TAU: Math.PI * 2, // for headless

  // javascript ////////////////////////////////////////////////////////

  is_empty: function (o)
  {
    return Object.keys(o).length === 0 && o.constructor === Object;
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
    while (a === 0) a = Math.random();
    while (b === 0) b = Math.random();
    let x = Math.sqrt(-2 * Math.log(a)) * Math.cos(UTIL.TAU * b);
    x = (x / 10) + 0.5;
    if (x > 1 || x < 0) return UTIL.normal(n);
    return x * n;
  },

  // TODO: this can be reconsidered and improved
  middle: function (n)
  {
    let factor = 4;
    return (Math.random() * n / factor) + (n / 2) - (n / 2 / factor);
  },

  // parameters ////////////////////////////////////////////////////////

  // Prepare state for core computation.
  // - Currently, this includes only converting the alpha and beta
  //   parameters into radians.
  prep: function (state)
  {
    let u = UTIL;
    let o = u.clone(state);
    if (o.a) o.a = u.rad(o.a);
    if (o.b) o.b = u.rad(o.b);
    return o;
  },

  // A predefined neighborhood radius value, if not provided by user.
  auto_radius: function (world, state)
  {
    return Math.round(Math.sqrt(world.w * world.h * state.den /
      state.num / Math.PI));
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
    let o = state.distr;
    console.log("EMERGENCE\n---------" +
                "\nwidth  = " + world.w +
                "\nheight = " + world.h +
                "\nstop   = " + state.stop +
                "\ndistr   = " + (o === 0 ? "uniform" : o === 1 ? "gaussian" : o === 2 ? "middle" : "ERROR") +
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

////////////////////////////////////////////////////////////////////////
// headless

function headless()
{
  return !(typeof window != "undefined" && window.document);
}

if (headless())
{
  module.exports =
  {
    is_empty: UTIL.is_empty,
    clone: UTIL.clone,
    mod: UTIL.mod,
    rad: UTIL.rad,
    deg: UTIL.deg,
    uniform: UTIL.uniform,
    gaussian: UTIL.gaussian,
    middle: UTIL.middle,
    prep: UTIL.prep,
    auto_radius: UTIL.auto_radius,
    for_abbrev: UTIL.for_abbrev,
    debug_params: UTIL.debug_params,
  };
}

