// EXECUTION:
// mono-csc ./*/*.cs ./Chesss.cs && mono Chesss.exe

using System;
using Chesss.Model;
using Chesss.Controller;
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
      Display display = new Display();

      Place place = new Place(2, 4);
      move.It(place, place.N());

      Console.WriteLine(display);
    }
  }
}
