import type { Confidence, EntryResult } from '../lib/api'

type OffsetResultsProps = {
  entries: EntryResult[]
  validated: number
  unverified: number
}

function formatRva(value: number | null) {
  if (value === null) return 'n/a'
  return `0x${value.toString(16).toUpperCase()}`
}

function toneFor(status: EntryResult['status']) {
  if (status === 'match') return 'text-apex-lime'
  if (status === 'moved') return 'text-apex-pink-soft'
  return 'text-apex-danger'
}

// The dot is the runtime check, not the signature hit: high means the global
// held an aligned pointer, medium an unaligned one, low/none a value we would
// not write into the dataset.
function dotClass(confidence: Confidence) {
  if (confidence === 'high') return 'bg-apex-lime shadow-[0_0_6px_var(--color-apex-lime)]'
  if (confidence === 'medium') return 'bg-apex-pink-soft shadow-[0_0_6px_var(--color-apex-pink-soft)]'
  if (confidence === 'low') return 'bg-apex-danger shadow-[0_0_6px_var(--color-apex-danger)]'
  return 'bg-white/25'
}

// One row per dataset entry: the entry name, its runtime confidence, and the
// RVA found in client.dll. Moved entries are pink, unresolved ones red.
export function OffsetResults({ entries, validated, unverified }: OffsetResultsProps) {
  return (
    <div className="flex h-full w-full flex-col gap-[3px]">
      <div className="flex shrink-0 items-center justify-between px-1 pb-px text-[7px] uppercase tracking-wide text-white/40">
        <span>{entries.length} entries</span>
        <span>
          {validated} verified{unverified > 0 ? ` · ${unverified} unverified` : ''}
        </span>
      </div>
      <div className="results-scroll flex min-h-0 w-full flex-col gap-[3px] overflow-y-auto">
        {entries.map((entry) => (
          <div
            key={entry.name}
            className="flex shrink-0 items-center justify-between rounded-md bg-white/[0.04] px-1.5 py-1"
            title={`${entry.status} | ${entry.confidence} confidence | stored ${formatRva(entry.storedRva)} | found ${formatRva(entry.foundRva)} | ${entry.validation}`}
          >
            <span className="flex min-w-0 items-center gap-1.5 pr-2">
              <span className={`h-[4px] w-[4px] shrink-0 rounded-full ${dotClass(entry.confidence)}`} />
              <span className="truncate text-[10px] text-white/80">{entry.name}</span>
            </span>
            <span className={`font-mono text-[10px] tabular-nums ${toneFor(entry.status)}`}>{formatRva(entry.foundRva)}</span>
          </div>
        ))}
      </div>
    </div>
  )
}
