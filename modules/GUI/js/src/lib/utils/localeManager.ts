/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

import {LocaleType} from '../messages/messages';

export type LocaleLayoutDirection = 'LTR' | 'RTL';

/**
 * Manages Locale state for web application layer.
 */
export class LocaleManager {
    private static locales : LocaleType[] = ['en-US'];
    private static localeLayoutDirection : LocaleLayoutDirection = 'LTR';

    public static setLocales(locales : LocaleType[]) {
        this.locales = locales;
        switch (this.getPrimaryLocale()) {
            case 'ar-SA':
                this.localeLayoutDirection = 'RTL';
                break;
            default:
                this.localeLayoutDirection = 'LTR';
                break;
        }
    }

    /**
     * Returns current array of LocaleType
     */
    public static getLocales() : LocaleType[] {
        return this.locales;
    }

    /**
     * Returns primary LocalType
     */
    public static getPrimaryLocale() : LocaleType {
        return this.locales.length > 0 ? this.locales[0] : undefined;
    }

    /**
     * Returns secondary LocalType
     */
    public static getSecondaryLocale() : LocaleType {
        return this.locales.length === 2 ? this.locales[1] : undefined;
    }

    /**
     * Returns @LocaleLayoutDirection for primary locale.
     */
    public static getLocaleLayoutDirection() : LocaleLayoutDirection {
        return this.localeLayoutDirection;
    }
}
