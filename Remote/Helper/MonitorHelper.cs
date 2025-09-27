using System.Diagnostics;

namespace MilkwaveRemote.Helper {
  internal static class MonitorHelper {
    // GPU counters
    private static Lazy<List<PerformanceCounter>> GpuCounters
        = new Lazy<List<PerformanceCounter>>(InitGpuCounters, true);

    // CPU counter
    private static Lazy<PerformanceCounter> CpuCounter
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

    // Returns the sum of NextValue() over all GPU engines (percent)
    public static float GetGPUUsage() {
      float res = -1f;
      try {
        var counters = GpuCounters.Value;
        // Sum across all engines
        res = counters.Sum(c => c.NextValue());
      } catch (Exception e) {
        GpuCounters = new Lazy<List<PerformanceCounter>>(InitGpuCounters, true);
      }
      return res;
    }

    // Returns current total CPU utilization (percent)
    public static float GetCPUUsage() {
      float res = -1f;
      try {
        res = CpuCounter.Value.NextValue();
      } catch (Exception e) {
        CpuCounter
        = new Lazy<PerformanceCounter>(InitCpuCounter, isThreadSafe: true);
      }
      return res;
    }
  }
}
