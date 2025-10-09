using GitHub;
using GitHub.Octokit.Client;
using Microsoft.Kiota.Abstractions.Authentication;

namespace CNC.NCO.Launcher.Service;

using Autofac;

public class ServicesModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    // needs called once per application lifetime
    DiscUtils.Complete.SetupHelper.SetupComplete();

    // BUG: without singleton scope RequestAdapter throws error on second instantiation
    //      (even if previous instance is disposed)
    var gitHubClient = new GitHubClient(
      RequestAdapter.Create(new AnonymousAuthenticationProvider() )
    );

    builder.RegisterInstance(gitHubClient).SingleInstance().AsSelf();

    builder.RegisterAssemblyTypes(ThisAssembly)
      .Where(t =>
        (t.Namespace?.StartsWith("CNC.NCO.Launcher.Service") ?? false) &&
        t is { IsClass: true, IsAbstract: false } &&
        t.Name.EndsWith("Service")
      )
      .AsSelf();
  }
}
