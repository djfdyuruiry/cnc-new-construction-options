using RestSharp;

namespace CNC.NCO.Launcher.Service;

using Autofac;

public class ServicesModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    // needs called once per application lifetime
    DiscUtils.Complete.SetupHelper.SetupComplete();

    var gitHubClient = new RestClient("https://api.github.com");;

    builder.RegisterInstance(gitHubClient).SingleInstance().As<IRestClient>();

    builder.RegisterAssemblyTypes(ThisAssembly)
      .Where(t =>
        (t.Namespace?.StartsWith("CNC.NCO.Launcher.Service") ?? false) &&
        t is { IsClass: true, IsAbstract: false } &&
        t.Name.EndsWith("Service")
      )
      .AsSelf();
  }
}
