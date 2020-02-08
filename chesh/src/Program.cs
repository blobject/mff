using System;
//using Chesh.Controller;
using Chesh.Model;
//using Chesh.View;

namespace Chesh
{
  class Program
  {
    static void
    Main(string[] args)
    {
      Game game = new Game();
      //game.Select(2, 2);
      Console.WriteLine(game.State.ToJson());
      //Move move = new Move(game);
      //Ui ui = new Ui(move);
      //ui.Run();
    }
  }
}
