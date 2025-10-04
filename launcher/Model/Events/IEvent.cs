namespace CNC.NCO.Launcher.Model.Events;

public interface IEvent<in T> where T : IVisitor
{
  void Accept(T visitor);
}
