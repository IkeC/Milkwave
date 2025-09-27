using System.Diagnostics;

namespace MilkwaveRemote.Helper {
  internal static class MonitorHelper {
    // GPU counters (cached once)
    private static readonly Lazy<List<PerformanceCounter>> _gpuCounters
        = new Lazy<List<PerformanceCounter>>(InitGpuCounters, isThreadSafe: true);

    // CPU counter (cached once)
    private static readonly Lazy<PerformanceCounter> _cpuCounter
        = new Lazy<PerformanceCounter>(InitCpuCounter, isThreadSafe: true);

    // Initialize and warm up GPU counters
    private static List<PerformanceCounter> InitGpuCounters() {
      var cat = new PerformanceCounterCategory("GPU Engine");
      var counters = cat
          .GetInstanceNames()
          .SelectMany(inst => cat.GetCounters(inst))
          .Where(c => c.CounterName == "Utilization Percentage")
          .ToList();

      // First NextValue often returns 0
      counters.ForEach(c => _ = c.NextValue());
      return counters;
    }

    // Initialize and warm up CPU counter
    private static PerformanceCounter InitCpuCounter() {
      var cpu = new PerformanceCounter("Processor", "% Processor Time", "_Total", readOnly: true);
      // First call returns 0, so prime it
      _ = cpu.NextValue();
      return cpu;
    }

    /// <summary>
    /// Returns the sum of NextValue() over all GPU engines (percent).
    /// </summary>
    public static float GetGPUUsage() {
      var counters = _gpuCounters.Value;
      // Sum across all engines
      return counters.Sum(c => c.NextValue());
    }

    /// <summary>
    /// Returns current total CPU utilization (percent).
    /// </summary>
    public static float GetCPUUsage() {
      return _cpuCounter.Value.NextValue();
    }
  }

}
