import { PrismaClient } from '@prisma/client';
import { executeNovaCommand } from '../cli/executor';
import path from 'path';
import fs from 'fs';

const prisma = new PrismaClient();
const BASE_STORAGE_PATH = process.env.NOVA_STORAGE_ROOT || '/tmp/nova_repos';

export class RepoService {
  static async createRepository(name: string, description?: string) {
    const repoPath = path.join(BASE_STORAGE_PATH, name);
    
    if (!repoPath.startsWith(BASE_STORAGE_PATH)) throw new Error("Invalid path");
    if (!fs.existsSync(repoPath)) fs.mkdirSync(repoPath, { recursive: true });

    const cliResult = await executeNovaCommand(repoPath, ['init']);
    if (!cliResult.success) throw new Error(`Nova init failed: ${cliResult.stderr}`);

    return prisma.repository.create({
      data: { name, path: repoPath, description }
    });
  }

  static async getAnalytics(id: string) {
    const repo = await prisma.repository.findUnique({ where: { id } });
    if (!repo) throw new Error("Repository not found");

    const cliResult = await executeNovaCommand(repo.path, ['analyze']);
    if (!cliResult.success) throw new Error("Failed to generate analytics");

    return JSON.parse(cliResult.stdout); 
  }
  
  static async makeCommit(id: string, message: string) {
    const repo = await prisma.repository.findUnique({ where: { id } });
    if (!repo) throw new Error("Repository not found");

    const cliResult = await executeNovaCommand(repo.path, ['commit', '-m', message]);
    if (!cliResult.success) throw new Error(cliResult.stderr);
    return cliResult.stdout;
  }
}
