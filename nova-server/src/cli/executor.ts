import { spawn } from 'child_process';
import { logger } from '../utils/logger';

export interface CliResult {
  success: boolean;
  exitCode: number | null;
  stdout: string;
  stderr: string;
}

export const executeNovaCommand = (
  repoPath: string,
  args: string[],
  timeoutMs: number = 10000
): Promise<CliResult> => {
  return new Promise((resolve) => {
    const executable = process.env.NOVA_EXECUTABLE_PATH || 'nova';
    logger.debug(`Executing: ${executable} ${args.join(' ')} in ${repoPath}`);

    const child = spawn(executable, args, { cwd: repoPath, timeout: timeoutMs });

    let stdout = '';
    let stderr = '';

    child.stdout.on('data', (data) => (stdout += data.toString()));
    child.stderr.on('data', (data) => (stderr += data.toString()));

    child.on('error', (err) => {
      logger.error(`CLI execution error: ${err.message}`);
      resolve({ success: false, exitCode: -1, stdout, stderr: err.message });
    });

    child.on('close', (code) => {
      resolve({
        success: code === 0,
        exitCode: code,
        stdout: stdout.trim(),
        stderr: stderr.trim(),
      });
    });
  });
};
