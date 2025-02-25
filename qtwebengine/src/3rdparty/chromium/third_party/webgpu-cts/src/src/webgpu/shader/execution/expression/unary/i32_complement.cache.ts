import { i32 } from '../../../../util/conversion.js';
import { fullI32Range } from '../../../../util/math.js';
import { makeCaseCache } from '../case_cache.js';

export const d = makeCaseCache('unary/i32_complement', {
  complement: () => {
    return fullI32Range().map(e => {
      return { input: i32(e), expected: i32(~e) };
    });
  },
});
