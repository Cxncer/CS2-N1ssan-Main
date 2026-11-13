type LiquidProgressProps = {
  percent: number
  label: string
}

// The fill height is real work progress reported by the Rust side, not a
// decorative animation.
export function LiquidProgress({ percent, label }: LiquidProgressProps) {
  const clamped = Math.max(0, Math.min(100, Math.round(percent)))

  return (
    <div className="pointer-events-none absolute inset-x-2 top-2 h-20 overflow-hidden rounded-[20px] bg-white/[0.04] ring-1 ring-white/5">
      <div
        className="liquid-fill absolute inset-x-0 bottom-0 bg-gradient-to-br from-[rgba(20,228,100,0.95)] to-[rgba(44,14,114,0.9)] shadow-[inset_0_1px_3px_rgba(255,255,255,0.45),0_0_16px_rgba(255,46,140,0.6)]"
        style={{ height: `${clamped}%` }}
      />
      {/* The dashboard card covers the lower part of this layer, so the readout
          sits in the strip that stays visible. */}
      <div className="absolute inset-x-0 top-1.5 z-10 flex flex-col items-center gap-0">
        <span className="text-sm font-bold tabular-nums tracking-tight text-white">{String(clamped).padStart(2, '0')}%</span>
        <span className="text-[7px] text-white/65">{label}</span>
      </div>
    </div>
  )
}
