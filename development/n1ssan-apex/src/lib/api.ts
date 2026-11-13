import { invoke } from '@tauri-apps/api/core'

// Mirrors the serde shapes in src-tauri/src/definitions.rs and lib.rs.

export type EntryStatus = 'match' | 'moved' | 'missing' | 'new'

export type EntryResult = {
  name: string
  module: string
  status: EntryStatus
  storedRva: number | null
  foundRva: number | null
}

export type ApexStatus = {
  gameRunning: boolean
  gamePid: number | null
  clientModule: boolean
  datasetPath: string
  datasetVersion: string
  datasetEntries: number
  appVersion: string
}

export type ScanReport = {
  entries: EntryResult[]
  total: number
  moved: number
  missing: number
  newCount: number
  datasetVersion: string
  datasetPath: string
  scannedAt: string
}

export type ApplyReport = {
  datasetPath: string
  backupPath: string | null
  datasetVersion: string
  changed: string[]
  updatedAt: string
}

export type ProgressEvent = {
  percent: number
  label: string
  message: string
}

export type ApexError = {
  code: string
  message: string
}

export function isApexError(value: unknown): value is ApexError {
  return typeof value === 'object' && value !== null && 'code' in value && 'message' in value
}

export function describeError(value: unknown): string {
  if (isApexError(value)) return `${value.code}: ${value.message}`
  if (value instanceof Error) return value.message
  return String(value)
}

export const apexStatus = () => invoke<ApexStatus>('apex_status')

export const apexScan = () => invoke<ScanReport>('apex_scan')

export const apexApply = () => invoke<ApplyReport>('apex_apply')
