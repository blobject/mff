// file: emergence/headless.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// headless (ie. running CORE from the commandline)

// Commandline argument handling.
function arg(args)
{
  const first = args[0];
  let exit = false;
  let file;
  let string;

  // Help
  if (first === "-?" || first === "-h" || first === "--help")
  {
    console.error("Usage:\n  node core.js [ -?/-h/--help | -f/--file FILE [STRING] | STRING ]\nParameters:\n  wid hei stop distr fps num psize rad den alpha beta gamma\n  [index x y phi]");
    exit = true;
  }

  // File input
  else if (first === "-f" || first === "--file")
  {
    const filename = args[1];
    console.error("Loading in file: " + filename + "\n");
    file = require("fs").readFileSync(filename, {encoding: "utf8"});

    // String input (also)
    if (args[3])
    {
      console.error("Too many arguments, considering only the file and string.\n");
    }
    string = args[2];
  }

  // String input (only)
  else
  {
    if (args[1])
    {
      console.error("Too many arguments, considering only the string.\n");
    }
    string = args[0];
  }

  return ({
    exit,
    file,
    string,
  });
}

// Count how many particles have number of neighbors in specific ranges:
// (0)--(X/2), (X/2)--(X), (X)--(3X/2), (3X/2)--(2X), (2X)--(4X), (4X)--
function analyse_crowd(WORLD, PTS)
{
  const num = PTS.length;
  const avg = WORLD.crowd;
  const crowds = [0, 0, 0, 0, 0, 0];
  const one = avg / 2;
  const two = avg;
  const tre = 3 * avg / 2;
  const fur = 2 * avg;
  const fiv = 4 * avg;
  let n;

  for (let i = 0; i < num; i++)
  {
    n = PTS[i].n;

    if      (n  < one)            crowds[0]++;
    else if (n >= one && n < two) crowds[1]++;
    else if (n >= two && n < tre) crowds[2]++;
    else if (n >= tre && n < fur) crowds[3]++;
    else if (n >= fur && n < fiv) crowds[4]++;
    else                          crowds[5]++;
  }

  for (let i = 0; i < crowds.length; i++)
  {
    crowds[i] = (crowds[i] * 100 / num).toFixed(2);
  }

  return crowds;
}

// Perform analyses and print their results.
function analyse(WORLD, PTS)
{
  const crowds = analyse_crowd(WORLD, PTS);

  console.error("\nMore data:\n- Average crowd (X): " +
                WORLD.crowd + "\n- Crowd distribution:" +
                "\n  -           N < (X/2) : " + crowds[0] +
                "%\n  -  (X/2) <= N < (X)   : " + crowds[1] +
                "%\n  -   (X)  <= N < (3X/2): " + crowds[2] +
                "%\n  - (3X/2) <= N < (2X)  : " + crowds[3] +
                "%\n  -  (2X)  <= N < (4X)  : " + crowds[4] +
                "%\n  -  (4X)  <= N         : " + crowds[5] + "%");
}

// Run headlessly.
function exec(CORE, WORLD, STATE)
{
  const opt = arg(process.argv.splice(2));

  if (opt.exit) { return; }

  // Default dimensions
  if (!STATE.width) { STATE.width = 1000; }
  if (!STATE.height) { STATE.height = 1000; }

  let raw = {};

  // Decode input
  if (!!opt.file) // file input
  {
    raw = CORE.decode(opt.file);
  }

  if (!!opt.string) // string input
  {
    raw = Object.assign(raw, CORE.decode(opt.string));
  }

  if (!opt.file && !opt.string) // no input
  {
    console.error("Using default values: w=" +
                  STATE.width + ", h=" + STATE.height + ", t=" +
                  STATE.stop + ", etc.\n");
    raw = CORE.get_params();
  }

  // Check and set input state
  CORE.set(CORE.sanitise(raw));

  // Create particles and set a posteriori parameters
  if (!(opt.file && !opt.string))
  {
    CORE.init(true); // create new particles
  }
  else
  {
    CORE.init(); // do not create new particles
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

  // Print final state
  console.error("done. (took " + (time[0] + (time[1] / 1e9))
    .toFixed(3) + " ms)\n");
  console.log(CORE.encode(STATE.width, STATE.height,
                          CORE.get_snapshot()));

  // Analyse
  analyse(WORLD, CORE.get_pts());
}

module.exports =
{
  exec,
};

