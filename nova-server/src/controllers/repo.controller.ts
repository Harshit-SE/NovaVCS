import { Request, Response } from 'express';
import { RepoService } from '../services/repo.service';
import { createRepoSchema, commitSchema } from '../validators/repo.validator';
import { logger } from '../utils/logger';

const formatResponse = (success: boolean, data: any, error: any = null) => ({ success, data, error });

export const createRepo = async (req: Request, res: Response) => {
  try {
    const { name, description } = createRepoSchema.parse(req.body);
    const repo = await RepoService.createRepository(name, description);
    res.status(201).json(formatResponse(true, repo));
  } catch (err: any) {
    logger.error(err);
    res.status(400).json(formatResponse(false, null, { message: err.message }));
  }
};

export const getAnalytics = async (req: Request, res: Response) => {
  try {
    const stats = await RepoService.getAnalytics(req.params.id);
    res.status(200).json(formatResponse(true, stats));
  } catch (err: any) {
    res.status(500).json(formatResponse(false, null, { message: err.message }));
  }
};

export const createCommit = async (req: Request, res: Response) => {
  try {
    const { message } = commitSchema.parse(req.body);
    const output = await RepoService.makeCommit(req.params.id, message);
    res.status(200).json(formatResponse(true, { output }));
  } catch (err: any) {
    res.status(400).json(formatResponse(false, null, { message: err.message }));
  }
};
