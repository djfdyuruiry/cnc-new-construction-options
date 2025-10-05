using System;

namespace CNC.NCO.Launcher.Model;

public class GameDataDownloadException : Exception
{
  public GameDataDownloadException(string? message) : base(message) {}

  public GameDataDownloadException(string? message, Exception? innerException) : base(message, innerException) {}
}
