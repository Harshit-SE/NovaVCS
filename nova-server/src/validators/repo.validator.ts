import { z } from 'zod';

export const createRepoSchema = z.object({
  name: z.string().min(1).max(100).regex(/^[a-zA-Z0-9-_]+$/, "Alphanumeric, dashes, and underscores only"),
  description: z.string().optional(),
});

export const commitSchema = z.object({
  message: z.string().min(1, "Commit message cannot be empty"),
});
