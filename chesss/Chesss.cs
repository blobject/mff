// EXECUTION (on linux):
// mono-csc ./*/*.cs ./Chesss.cs -out:Chesss.exe && mono Chesss.exe

using System;
using Chesss.Controller;
using Chesss.Model;
using Chesss.View;

namespace Chesss
{
  class Program
  {
    static void
    Main(string[] args)
    {
      Game game = new Game();
      Move move = new Move(game);
      Ui ui = new Ui(move);
      ui.Run();
    }
  }
}
