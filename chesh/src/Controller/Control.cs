using System;
using System.Collections.Generic;
using Chesh.Model;
using Chesh.Util;
using Chesh.View;

namespace Chesh.Controller
{
  public class Control : IObserver
  {
    private Game Game;
    private Ui Ui;

    public Control(Game game, Ui ui)
    {
      this.Game = game;
      this.Ui = ui;
    }

    // M -> C (-> set V) ///////////////////////////////////////////////////////

    public void
    ChangeState(State next)
    {
      this.Ui.Restate(Helper.ToJson(next));
      this.Ui.Draw();
    }

    // V -> C (-> set M) ///////////////////////////////////////////////////////

    public List<Ret>
    Move(int xSrc, int ySrc, int xDst, int yDst)
    {
      return this.Game.Move(xSrc, ySrc, xDst, yDst);
    }

    public void
    Promote(List<Ret> rets, string prom, int xSrc, int ySrc, int xDst, int yDst)
    {
      this.Game.Promote(rets, prom, xSrc, ySrc, xDst, yDst);
    }

    // TODO: unused
    public Ret
    Select(int x, int y)
    {
      return this.Game.Select(x, y);
    }

    public void
    Save()
    {
      this.Game.Save();
    }
  }
}
