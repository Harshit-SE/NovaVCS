import { executeNovaCommand } from '../src/cli/executor';

describe('NovaVCS CLI Executor', () => {
  it('should safely execute commands and return structured results', async () => {
    process.env.NOVA_EXECUTABLE_PATH = 'echo';
    const result = await executeNovaCommand('/tmp', ['test', 'output']);
    
    expect(result.success).toBe(true);
    expect(result.exitCode).toBe(0);
    expect(result.stdout).toBe('test output');
  });
});
