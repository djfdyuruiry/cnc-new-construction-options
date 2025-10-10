using System;
using System.Diagnostics;
using System.Linq;

namespace CNC.NCO.Launcher.Model.Util;

public class ProcessExecuteResult(Process? process)
{
  public ProcessExecuteResult() : this(null) {}

  public int ExitCode { get; } = process?.ExitCode ?? -1;
  public string? StdOut { get; } = process?.StandardOutput.ReadToEnd();
  public string? StdErr { get; } = process?.StandardError.ReadToEnd();
  public bool CreateProcessFailed { get; } = process is null;

  public void AssertExitCode(string message, int exitCode = 0)
  {
    if (ExitCode != exitCode)
    {
      throw new Exception($"{message}\n{ToString()}");
    }
  }

  public override string ToString()
  {
    return string.Join(
      "\n",
      GetType().GetProperties().Select(p => $"{p.Name}: {p.GetValue(this)}")
    );
  }
}
