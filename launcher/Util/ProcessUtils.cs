using System;
using System.Diagnostics;
using System.Threading.Tasks;

using CNC.NCO.Launcher.Model.Util;

namespace CNC.NCO.Launcher.Util;

public static class ProcessUtils
{
  public static ProcessStartInfo BuildStartInfo(string binaryPath, params string[] arguments) =>
    new()
    {
      FileName = binaryPath,
      Arguments = string.Join(" ", arguments),
      CreateNoWindow = true,
      RedirectStandardOutput = true,
      RedirectStandardError = true
    };

  public static async Task<ProcessExecuteResult> Exec(ProcessStartInfo startInfo)
  {
    using var process = Process.Start(startInfo);

    if (process is not null)
    {
      await process.WaitForExitAsync();
    }

    return new ProcessExecuteResult(process);
  }

  public static async Task<ProcessExecuteResult> Exec(string binaryPath, params string[] arguments) =>
    await Exec(BuildStartInfo(binaryPath, arguments));

  public static Process? ExecWithCallback(ProcessStartInfo startInfo, Action<ProcessExecuteResult> onExit)
  {
    var process = new Process()
    {
      EnableRaisingEvents = true,
      StartInfo = startInfo
    };

    // Reason: process is only disposed if process fails to start
    // ReSharper disable once AccessToDisposedClosure
    process.Exited += (_, _) => onExit(new ProcessExecuteResult(process));

    if (!process.Start())
    {
      onExit(new ProcessExecuteResult());

      process.Dispose();
      return null;
    }

    return process;
  }

  public static Process? ExecWithCallback(
    string binaryPath,
    Action<ProcessExecuteResult> onExit,
    params string[] arguments
  ) => ExecWithCallback(BuildStartInfo(binaryPath, arguments), onExit);
}
