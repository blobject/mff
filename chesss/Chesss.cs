// EXECUTION (on linux):
// (use .Net Core due to segfault on ubuntu & debian:
//  https://dotnetcli.blob.core.windows.net/dotnet/Sdk/master/dotnet-sdk-latest-linux-x64.tar.gz
//  see also: https://github.com/dotnet/runtime/issues/460
//            https://github.com/dotnet/core/issues/973)
// cd mff/chesss/
// (make sure Chesss.csproj.TargetFramework is netcoreapp5.0)
// dotnet run

using System;
//using Chesss.Controller;
using Chesss.Model;
//using Chesss.View;

namespace Chesss
{
  class Program
  {
    static void
    Main(string[] args)
    {
      Game game = new Game();
      Console.WriteLine(game.State.ToJson());
      //Move move = new Move(game);
      //Ui ui = new Ui(move);
      //ui.Run();
    }
  }
}
