// file: emergence/headless.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// headless (ie. running CORE from the commandline)

// Commandline argument handling.
function opt(args)
{
  const first = args[0];
  let act;
  let res;

  // Help
  if (first === "-?" || first === "-h" || first === "--help")
  {
    act = 1;
    res = "usage:\n  node core.js [ -?/-h/--help | -f/--file FILE | STRING ]\n";
  }

  // File input
  else if (first === "-f" || first === "--file")
  {
    const file = args[1];
    act = 2;
    console.error("Loading in file: " + file + "\n");
    res = require("fs").readFileSync(file, {encoding: "utf8"});
  }

  // String input
  else
  {
    if (args[1])
    {
      console.error("Too many arguments, ignoring everything but first.\n");
    }
    act = 2;
    res = args[0];
  }

  return [act, res];
}

// Run headlessly.
function exec(CORE, WORLD, STATE)
{
  const act = opt(process.argv.splice(2));
  const input = act[1];

  // Help
  if (act[0] === 1)
  {
    console.log(input);
  }

  // Okay, run CORE
  else if (act[0] === 2)
  {
    if (!STATE.w) { STATE.w = 1000; }
    if (!STATE.h) { STATE.h = 1000; }

    let raw;

    // If either file or string input was provided
    if (!!input)
    {
      raw = CORE.decode(input);
    }

    // If no input was provided, set a default stop value
    else
    {
      CORE.init(true);
      raw = CORE.get_params();
    }

    CORE.set(CORE.sanitise(raw));

    if (!input)
    {
      console.error("Using default values: w=" +
                    STATE.w + ", h=" + STATE.h + ", t=" +
                    STATE.stop + ", etc.:\n");
    }

    let snap = CORE.get_snapshot();
    delete snap.p;
    process.stderr.write("Running with parameters:\n" +
                         JSON.stringify(snap, null, 2) +
                         "\n\nCalculating state after " +
                         STATE.stop + " ticks ... ");

    // Run
    let time = process.hrtime();
    while (WORLD.tick < STATE.stop)
    {
      WORLD.tick++;
      CORE.next(CORE, WORLD);
    }
    time = process.hrtime(time);

    // Output
    console.error("done. (took " + (time[0] + (time[1] / 1e9))
      .toFixed(3) + " ms)\n");
    console.log(CORE.encode(STATE.w, STATE.h, CORE.get_snapshot()));
  }

  // Should never come to this
  else
  {
    console.error("Incomprehensible arguments; don't know what to do.\n");
  }
}

module.exports =
{
  exec,
};

